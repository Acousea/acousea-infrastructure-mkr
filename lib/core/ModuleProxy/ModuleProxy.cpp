#include "ModuleProxy.hpp"

#include <cstdio>

#include "BinaryFrame/BinaryFrame.hpp"
#include "Logger/Logger.h"
#include "SharedMemory/SharedMemory.hpp"


// ===================================================
// ================ Constructor ======================
// ===================================================

constexpr const char* ModuleProxy::toString(const ModuleProxy::DeviceAlias alias) noexcept
{
    switch (alias)
    {
    case ModuleProxy::DeviceAlias::PIDevice: return "PIDevice";
    case ModuleProxy::DeviceAlias::VR2C: return "VR2C";
    default: return "Unknown";
    }
}

#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED
ModuleProxy::ModuleProxy(Router& router, const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap)
    : router(router), devicePortMap(devicePortMap)
{
}

bool ModuleProxy::begin()
{
    return true;
}

#else
ModuleProxy::ModuleProxy(
    Router& router,
    const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap,
    StorageManager& storageManager,
    RTCController& rtcController
) :
    router(router),
    devicePortMap(devicePortMap),
    storage_(storageManager),
    rtc_(rtcController)
{
}

bool ModuleProxy::begin()
{
    // Initialize storage manager if needed
    if (const bool beginOk = storage_.begin(); !beginOk)
    {
        LOG_CLASS_ERROR("::begin() -> Failed to initialize StorageManager");
        return false;
    }

    constexpr auto MIN_CODE = _acousea_ModuleCode_MIN + 1;
    constexpr auto MAX_CODE = _acousea_ModuleCode_MAX;
    for (auto code = MIN_CODE; code <= MAX_CODE; code++)
    {
        char path[10];
        snprintf(path, sizeof(path), "/mod%d", static_cast<int>(code));

        if (storage_.fileExists(path))
        {
            LOG_CLASS_INFO("::begin() -> Module cache file exists for module %d", static_cast<int>(code));
            readOffset_[code] = storage_.fileSize(path);
            writeOffset_[code] = storage_.fileSize(path);
        }
        else
        {
            if (const bool createOk = storage_.createEmptyFile(path); !createOk)
            {
                LOG_CLASS_ERROR("::begin() -> Cannot create module cache file: %s", path);
                return false;
            }
            LOG_CLASS_INFO("::begin() -> Created module cache file: %s", path);
        }
    }
    return true;
}

#endif


// ===================================================
// ============== Envío de comandos ==================
// ===================================================

bool ModuleProxy::requestMultipleModules(const acousea_ModuleCode* codes,
                                         const pb_size_t count,
                                         const DeviceAlias alias)
{
    acousea_CommunicationPacket& pkt = buildRequestModulePacket(codes, count);
    const auto portType = resolvePort(alias);
    if (portType == IPort::PortType::None)
    {
        LOG_CLASS_WARNING("ModuleProxy::requestMultipleModules() -> Could not resolve port for alias %s",
                          toString(alias));
        return false;
    }

    invalidateMultiple(codes, count); // The modules will be marked as fresh when the response is received
    LOG_CLASS_INFO("Requesting %d modules through %s for alias %s", static_cast<int>(count),
                   IPort::portTypeToCString(portType), toString(alias)
    );
    return router
           .from(Router::broadcastAddress)
           .through(portType)
           .send(pkt);
}

bool ModuleProxy::requestModule(const acousea_ModuleCode code, const DeviceAlias alias)
{
    acousea_CommunicationPacket& pkt = buildRequestModulePacket(&code, 1);
    const auto portType = resolvePort(alias);
    if (portType == IPort::PortType::None)
    {
        Logger::logWarning("ModuleProxy::requestModule() -> Could not resolve port for alias");
        return false;
    }

    // The module will be marked as not fresh when the response is received
    invalidateModule(code);

    LOG_CLASS_INFO("Requesting module %d through %s for alias %s", static_cast<int>(code),
                   IPort::portTypeToCString(portType), toString(alias)
    );

    return router
           .from(Router::broadcastAddress)
           .through(portType)
           .send(pkt);
}

bool ModuleProxy::sendModule(const acousea_ModuleWrapper& module,
                             const DeviceAlias alias)
{
    const auto moduleCodeUint = module.which_module;
    if (moduleCodeUint < _acousea_ModuleCode_MIN + 1 || moduleCodeUint > _acousea_ModuleCode_MAX)
    {
        LOG_CLASS_WARNING("ModuleProxy::sendModule() -> Invalid module code %d in ModuleWrapper",
                          static_cast<int>(moduleCodeUint));
        return false;
    }
    const auto code = static_cast<acousea_ModuleCode>(module.which_module);

    acousea_CommunicationPacket& pkt = buildSetModulePacket(code, module);

    const auto portType = resolvePort(alias);
    if (portType == IPort::PortType::None)
    {
        LOG_CLASS_WARNING("ModuleProxy::sendModule() -> Could not resolve port for alias %s", toString(alias));
        return false;
    }

    // The module must be previously invalidated and set not fresh
    invalidateModule(code);

    LOG_CLASS_INFO("::sendModule() -> Sending module %d through %s for alias %s", static_cast<int>(code),
                   IPort::portTypeToCString(portType), toString(alias)
    );


    return router
           .from(Router::broadcastAddress)
           .through(portType)
           .send(pkt);
}

IPort::PortType ModuleProxy::resolvePort(const DeviceAlias alias) const noexcept
{
    const auto it = devicePortMap.find(alias);
    if (it == devicePortMap.end())
    {
        LOG_CLASS_WARNING(
            "ModuleProxy::resolvePort() -> Node has no port associated with device alias: %s",
            toString(alias)
        );
        return IPort::PortType::None;
    }
    return it->second;
}


// Build the request packet using SharedMemory's communicationPacketRef()
acousea_CommunicationPacket& ModuleProxy::buildRequestModulePacket(const acousea_ModuleCode* codes,
                                                                   const pb_size_t count)
{
    LOG_CLASS_FREE_MEMORY("::buildRequestPacket(start multi)");
    auto& pkt = SharedMemory::communicationPacketRef(); // Use SharedMemory's communication packet
    pkt = acousea_CommunicationPacket_init_default; // Initialize the packet
    pkt.has_routing = true;
    pkt.routing.sender = Router::broadcastAddress;
    pkt.routing.receiver = 0; // device
    pkt.routing.ttl = 5;

    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_requestedConfiguration_tag;

    auto& req = pkt.body.command.command.requestedConfiguration;
    req = acousea_GetUpdatedNodeConfigurationPayload_init_default;

    // Limitar count al máximo permitido por nanopb
    constexpr pb_size_t maxModules = _acousea_ModuleCode_MAX;
    const pb_size_t safeCount = (count > maxModules) ? maxModules : count;
    req.requestedModules_count = safeCount;
    for (pb_size_t i = 0; i < safeCount; i++)
    {
        req.requestedModules[i] = codes[i];
    }

    LOG_CLASS_FREE_MEMORY(
        "::buildRequestPacket(end multi) -> requested %d modules (max allowed %d)",
        safeCount,
        maxModules
    );

    return pkt;
}

// ===================================================
// ============ buildSetPacket (corregido) ===========
// ===================================================

acousea_CommunicationPacket& ModuleProxy::buildSetModulePacket(acousea_ModuleCode code,
                                                               const acousea_ModuleWrapper& module)
{
    auto& pkt = SharedMemory::communicationPacketRef(); // Usamos el paquete de SharedMemory
    pkt = acousea_CommunicationPacket_init_default; // Inicializamos el paquete en SharedMemory
    pkt.has_routing = true;
    pkt.routing.sender = Router::broadcastAddress;
    pkt.routing.receiver = 0; // backend
    pkt.routing.ttl = 5;

    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;
    pkt.body.command.command.setConfiguration.modules_count = 1;

    auto& entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.has_value = true;
    entry.key = code;
    entry.value = module;

    return pkt;
}

const acousea_ModuleWrapper* ModuleProxy::getIfFreshOrRequestFromDevice(
    const acousea_ModuleCode code, const DeviceAlias alias
)
{
    const acousea_ModuleWrapper* optModulePtr = getIfFresh(code);

    if (optModulePtr != nullptr)
    {
        return optModulePtr;
    }

    if (const auto reqOk = requestModule(code, alias); !reqOk)
    {
        LOG_CLASS_WARNING("ModuleProxy::getIfFreshOrRequestTo() -> Failed to request module %d to %s",
                          static_cast<int>(code), toString(alias));
        return nullptr;
    }

    LOG_CLASS_INFO("ModuleProxy::getIfFreshOrRequestTo() -> Requested module %d to %s (waiting response)",
                   static_cast<int>(code), toString(alias));

    return nullptr;
}

const acousea_ModuleWrapper* ModuleProxy::getIfFreshOrSetOnDevice(
    const acousea_ModuleCode code,
    const acousea_ModuleWrapper& module,
    const DeviceAlias alias
)
{
    const acousea_ModuleWrapper* optModulePtr = getIfFresh(code);

    if (optModulePtr != nullptr)
    {
        // Módulo confirmado en caché, no hace falta enviar nada
        return optModulePtr;
    }

    if (const bool sendOk = sendModule(module, alias); !sendOk)
    {
        LOG_CLASS_ERROR("ModuleProxy::getIfFreshOrSetOnDevice() -> Failed to send module %d to %s",
                        static_cast<int>(code), toString(alias));
        return nullptr;
    }

    LOG_CLASS_INFO("ModuleProxy::getIfFreshOrSetOnDevice() -> Sent module %d to %s (waiting confirmation)",
                   static_cast<int>(code), toString(alias));

    return nullptr;
}

// =============================================================================================
// ================================ CACHE IN RAM IMPLEMENTATION ================================
// =============================================================================================

#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED
bool ModuleProxy::storeModule(const acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper)
{
    const uint16_t idx = static_cast<uint16_t>(code) - 1;
    if (idx < 0 || idx > ModuleProxy::MAX_MODULES - 1)
    {
        LOG_CLASS_WARNING("::storeModule() -> Module idx %d out of bounds [0, %d)",
                          static_cast<int>(idx), ModuleProxy::MAX_MODULES - 1);
        return false; // Código inválido
    }
    entries[idx].emplace(wrapper);
    return true; // No hay espacio
}

const acousea_ModuleWrapper* ModuleProxy::getIfFresh(acousea_ModuleCode code)
{
    const auto& entry = entries[static_cast<uint16_t>(code) - 1];
    if (entry.has_value())
    {
        return &entry.value();
    }
    return nullptr;
}

void ModuleProxy::invalidateModule(const acousea_ModuleCode code)
{
    const uint16_t idx = static_cast<uint16_t>(code) - 1;
    if (idx < 0 || idx > ModuleProxy::MAX_MODULES - 1)
    {
        LOG_CLASS_WARNING("::invalidateModule() -> Module idx %d out of bounds [0, %d)",
                          static_cast<int>(idx), ModuleProxy::MAX_MODULES - 1);
        return; // Código inválido
    }
    entries[idx] = (std::nullopt);
}

void ModuleProxy::invalidateAll()
{
    for (auto& e : entries)
    {
        e = std::nullopt;
    }
}

#else

bool ModuleProxy::storeMultipleModules(const acousea_ModuleWrapper* const* wrappers, const pb_size_t count)
{
    for (pb_size_t i = 0; i < count; i++)
    {
        if (!storeModule(*wrappers[i]))
        {
            LOG_CLASS_WARNING("::storeMultipleModules() -> Failed to store module %d", wrappers[i]->which_module);
            return false;
        }
    }
    return true;
}

bool ModuleProxy::storeModule(const acousea_ModuleWrapper& wrapper)
{
    // Archivo del módulo
    const auto wrapperCode = static_cast<acousea_ModuleCode>(wrapper.which_module);
    if (wrapperCode < _acousea_ModuleCode_MIN + 1 || wrapperCode > _acousea_ModuleCode_MAX)
    {
        LOG_CLASS_WARNING("::storeMultipleModules() -> Invalid module code %d in ModuleWrapper",
                          static_cast<int>(wrapperCode));
        return false;
    }

    char filePath[10];
    std::snprintf(filePath, sizeof(filePath), "/mod%d", static_cast<int>(wrapperCode));

    // Buffer temporal compartido
    auto* tmpEncodingBuffer = SharedMemory::tmpBuffer();
    constexpr size_t tmpEncodingBufferMaxSize = SharedMemory::tmpBufferSize();

    // Codificamos el wrapper dentro de los límites del buffer
    Result<size_t> encodeResultWithEncodedLength = ProtoUtils::ModuleWrapper::encodeInto(
        wrapper,
        tmpEncodingBuffer + BinaryFrame::HEADER_SIZE, // Leave space for BinaryFrame header
        tmpEncodingBufferMaxSize - BinaryFrame::HEADER_SIZE - BinaryFrame::FOOTER_SIZE
        // Leave space for header and footer
    );

    if (!encodeResultWithEncodedLength.isSuccess())
    {
        LOG_CLASS_WARNING("::storeModule() -> Failed to encode module %d", static_cast<int>(wrapperCode));
        return false;
    }

    const auto payloadLen = static_cast<uint16_t>(encodeResultWithEncodedLength.getValue());
    const size_t totalRecordSize = BinaryFrame::requiredSize(payloadLen); // START + LEN + PAYLOAD + END

    if (totalRecordSize > tmpEncodingBufferMaxSize)
    {
        LOG_CLASS_ERROR("::storeModule() -> Module record too large (%u bytes)", totalRecordSize);
        return false;
    }

    // --- CABECERA con BinaryFrame ---
    const uint32_t timestamp = rtc_.getEpoch();
    auto* payloadPtr = tmpEncodingBuffer + BinaryFrame::HEADER_SIZE; // Payload comienza después de la cabecera

    if (!BinaryFrame::wrapInPlace(
            tmpEncodingBuffer, // The out buffer which will contain the full wrapped frame
            tmpEncodingBufferMaxSize, // The max size of the out buffer
            payloadPtr, // The payload ptr, which is already written in place
            payloadLen, // The payload length
            timestamp) // The timestamp
    )
    {
        LOG_CLASS_WARNING("::storeModule() -> Failed to wrap module %d", static_cast<int>(wrapperCode));
        return false;
    }

    // --- APPEND A SD ---
    if (const bool appendOk = storage_.appendBytesToFile(filePath, tmpEncodingBuffer, totalRecordSize); !appendOk)
    {
        LOG_CLASS_WARNING("::storeModule() -> Failed to write module %d to file %s",
                          static_cast<int>(wrapperCode),
                          filePath);
        return false;
    }
    // First we need to update the read offset to the current write offset, to avoid reading more data from previous writes
    readOffset_[static_cast<int>(wrapperCode)] = writeOffset_[static_cast<int>(wrapperCode)];

    // Then we update the write offset to account for the newly written data
    writeOffset_[static_cast<int>(wrapperCode)] += totalRecordSize;

    LOG_CLASS_INFO("::storeModule() -> Stored module %d (%u bytes) in %s",
                   static_cast<int>(wrapperCode),
                   totalRecordSize,
                   filePath);

    return true;
}

bool ModuleProxy::isModuleFresh(const acousea_ModuleCode code) const
{
    const int idx = static_cast<int>(code);
    return readOffset_[idx] < writeOffset_[idx];
}

const acousea_ModuleWrapper* ModuleProxy::getIfFresh(const acousea_ModuleCode code)
{
    const int idx = static_cast<int>(code);

    // Si ya está cargado en RAM → devolverlo
    if (optLoadedModule_.has_value() && optLoadedModule_->which_module == code)
        return &optLoadedModule_.value();

    // Si no hay datos nuevos → no está fresco
    if (readOffset_[idx] == writeOffset_[idx])
    {
        LOG_CLASS_INFO("::getIfFresh() -> Module %d not fresh (readOffset == writeOffset)", idx);
        return nullptr;
    }

    char filePath[10];
    std::snprintf(filePath, sizeof(filePath), "/mod%d", idx);

    if (!storage_.fileExists(filePath))
    {
        LOG_CLASS_INFO("::getIfFresh() -> Module file %s does not exist", filePath);
        return nullptr;
    }

    // Buffer temporal
    auto* readBuf = SharedMemory::tmpBuffer();
    constexpr size_t readBufMaxSize = SharedMemory::tmpBufferSize();

    // --- Leer desde almacenamiento ---
    const size_t bytesRead = storage_.readFileRegionBytes(
        filePath,
        readOffset_[idx],
        readBuf,
        readBufMaxSize
    );

    if (bytesRead == 0)
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Failed to read module %d from file %s", idx, filePath);
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    // --- Unwrap ---
    BinaryFrame::FrameView outFrameView{};
    if (!BinaryFrame::unwrap(readBuf, bytesRead, outFrameView))
    {
        LOG_CLASS_WARNING("::getIfFresh() -> unwrap failed for module %d", idx);
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    // Sanity check: payload fits buffer
    const size_t totalFrameSize = BinaryFrame::requiredSize(outFrameView.payloadLength);
    if (bytesRead < totalFrameSize)
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Incomplete frame read for module %d (read %u < needed %u)",
                          idx, static_cast<unsigned int>(bytesRead), static_cast<unsigned int>(totalFrameSize));
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    // --- Decodificar directamente dentro del optional ---
    optLoadedModule_.emplace(); // Crear el optional vacío
    const Result<void> decodeResult = ProtoUtils::ModuleWrapper::decodeInto(
        outFrameView.payload, // Puntero directo al payload
        outFrameView.payloadLength, // Longitud exacta del payload
        &optLoadedModule_.value()
    );

    if (!decodeResult.isSuccess())
    {
        LOG_CLASS_WARNING("::getIfFresh() -> decodeInto failed for module %d", idx);
        optLoadedModule_.reset();
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    LOG_CLASS_INFO("::getIfFresh() -> Loaded module %d (ts=%lu, size=%u) nextOffset=%llu",
                   idx,
                   static_cast<unsigned long>(outFrameView.timestamp),
                   outFrameView.payloadLength,
                   static_cast<unsigned long long>(readOffset_[idx]));

    return &optLoadedModule_.value();
}


void ModuleProxy::invalidateMultiple(const acousea_ModuleCode* codes, const pb_size_t count)
{
    const pb_size_t safeCount = (count > _acousea_ModuleCode_MAX) ? _acousea_ModuleCode_MAX : count;
    for (pb_size_t i = 0; i < safeCount; i++)
    {
        invalidateModule(codes[i]);
    }
}

void ModuleProxy::invalidateModule(const acousea_ModuleCode code)
{
    const int idx = code;
    if (idx < _acousea_ModuleCode_MIN + 1 | idx > _acousea_ModuleCode_MAX)
    {
        LOG_CLASS_WARNING("::invalidateModule() -> Module code %d out of bounds", static_cast<int>(code));
        return; // Código inválido
    }

    // Marcamos como “no fresco”
    readOffset_[idx] = writeOffset_[idx];

    // Si había un módulo cargado, lo invalidamos también
    optLoadedModule_.reset();

    LOG_CLASS_INFO("::invalidateModule() -> Invalidated module %d (readOffset=%lu%lu, writeOffset=%lu%lu)",
                   idx,
                   static_cast<unsigned long>(readOffset_[idx] >> 32),
                   static_cast<unsigned long>(readOffset_[idx] & 0xFFFFFFFFULL),
                   static_cast<unsigned long>(writeOffset_[idx] >> 32),
                   static_cast<unsigned long>(writeOffset_[idx] & 0xFFFFFFFFULL)
    );
}


void ModuleProxy::invalidateAll()
{
    constexpr auto minCode = _acousea_ModuleCode_MIN + 1;
    constexpr auto maxCode = _acousea_ModuleCode_MAX;
    for (uint8_t code = minCode; code <= maxCode; code++)
    {
        invalidateModule(static_cast<acousea_ModuleCode>(code));
    }
}


#endif
