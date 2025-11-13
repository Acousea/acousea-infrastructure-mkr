#ifndef ACOUSEA_INFRASTRUCTURE_MKR_PENDINGROUTINES_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_PENDINGROUTINES_HPP

#include <optional>
#include <string>
#include <cstdio>
#include <vector>
#include <IRoutine.h>
#include <Ports/IPort.h>
#include <StorageManager/StorageManager.hpp>


#include "SharedMemory/SharedMemory.hpp"
#include "ProtoUtils/ProtoUtils.hpp"

template <size_t MAX_ROUTINES>
class PendingRoutines
{
public:
    using Packet = acousea_CommunicationPacket;
    using Routine = IRoutine<Packet>;

    struct Entry
    {
        Routine* routine = nullptr;
        Packet* packet = nullptr; // apuntará a SharedMemory
        char storagePath[24] = {}; // ruta corta estática (evita std::string)
        uint8_t remainingAttempts = 0;
        IPort::PortType portResponseTo{IPort::PortType::None};
    };

    explicit PendingRoutines(StorageManager& storage)
        : storage_(storage), head(0), tail(0), count(0)
    {
    }

    /** Limpia la carpeta /.cache/ al iniciar el sistema */
    void initialize()
    {
        char path[24];
        for (size_t i = 0; i < MAX_ROUTINES; ++i)
        {
            snprintf(path, sizeof(path), "/.cache/.pr_%zu", i);
            storage_.deleteFile(path);
        }
        head = tail = count = 0;
    }

    /** Añade una rutina pendiente al almacenamiento persistente */
    bool add(Routine* routine, const Packet& packet, IPort::PortType portResponseTo)
    {
        char path[24];
        snprintf(path, sizeof(path), "/.cache/.pr_%zu", tail);

        // 1. Encodear el paquete protobuf
        const auto encodedResult = ProtoUtils::CommunicationPacket::encode(packet);
        if (!encodedResult.isSuccess())
            return false;

        const auto& data = encodedResult.getValueConst();

        // 2. Guardar en almacenamiento persistente
        if (!storage_.writeFileBytes(path, data.data(), data.size()))
            return false;

        // 3. Crear entrada de metadatos
        Entry e{};
        e.routine = routine;
        strncpy(e.storagePath, path, sizeof(e.storagePath) - 1);
        e.portResponseTo = portResponseTo;
        e.remainingAttempts = 0;
        e.packet = nullptr;

        buffer[tail] = e;
        tail = (tail + 1) % MAX_ROUTINES;

        if (count >= MAX_ROUTINES)
        {
            storage_.deleteFile(buffer[head].storagePath);
            head = (head + 1) % MAX_ROUTINES;
        }
        else
        {
            ++count;
        }

        return true;
    }

    /**
    * Carga la siguiente rutina pendiente desde almacenamiento.
    * Decodifica el paquete directamente en SharedMemory::communicationPacket.
    * Devuelve puntero válido hasta la siguiente llamada a next().
    */
    Entry* next()
    {
        if (count == 0)
            return nullptr;

        Entry* e = &buffer[head];

        // Usar el buffer temporal global en lugar de vector dinámico
        char* tmpBuf = SharedMemory::tmpBuffer();
        constexpr size_t tmpBufSize = SharedMemory::tmpBufferSize();
        SharedMemory::clearTmpBuffer();

        // Leer archivo desde SD directamente en el buffer global
        const size_t readBytes = storage_.readFileBytes(
            e->storagePath,
            reinterpret_cast<uint8_t*>(tmpBuf),
            tmpBufSize);

        // Eliminar el archivo tras leerlo
        storage_.deleteFile(e->storagePath);

        if (readBytes == 0)
        {
            e->packet = nullptr;
            advanceHead();
            return e; // puntero válido hasta próxima llamada
        }

        // Decodificar directamente desde el buffer global
        const auto decodedResult = ProtoUtils::CommunicationPacket::decodeInto(
            reinterpret_cast<const uint8_t*>(tmpBuf),
            readBytes,
            &SharedMemory::communicationPacketRef()
        );

        if (!decodedResult.isSuccess())
        {
            e->packet = nullptr;
            advanceHead();
            return e;
        }

        e->packet = &SharedMemory::communicationPacketRef();
        advanceHead();
        return e;
    }

    [[nodiscard]] bool empty() const { return count == 0; }
    [[nodiscard]] size_t size() const { return count; }

    static constexpr const uint8_t MAX_ATTEMPTS = 3;

private:
    void advanceHead()
    {
        head = (head + 1) % MAX_ROUTINES;
        --count;
    }

    StorageManager& storage_;
    Entry buffer[MAX_ROUTINES];
    size_t head;
    size_t tail;
    size_t count;
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_PENDINGROUTINES_HPP
