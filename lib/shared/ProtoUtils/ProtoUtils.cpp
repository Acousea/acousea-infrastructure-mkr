#include "ProtoUtils.hpp"

#include <pb_encode.h>
#include <pb_decode.h>

#include "Logger/Logger.h"
#include "ClassName.h"

namespace ProtoUtils
{
    namespace CommunicationPacket
    {
        CLASS_NAME(ProtoUtils::CommunicationPacket)

        // ================================ FROM BUFFER ================================

        // Decodifica un buffer raw -> acousea_CommunicationPacket
        Result<acousea_CommunicationPacket> decode(const std::vector<uint8_t>& raw)
        {
            if (raw.empty())
            {
                return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "decodePacket: empty input buffer");
            }

            acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;

            pb_istream_t is = pb_istream_from_buffer(raw.data(), raw.size());
            if (!pb_decode(&is, acousea_CommunicationPacket_fields, &pkt))
            {
                return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "decodePacket: pb_decode failed: %s",
                                             PB_GET_ERROR(&is));
            }

            return RESULT_SUCCESS(acousea_CommunicationPacket, pkt);
        }

        Result<void> decodeInto(const uint8_t* data, size_t length, acousea_CommunicationPacket* out)
        {
            if (out == nullptr)
            {
                return RESULT_VOID_FAILUREF("decodeInto: destination pointer is null");
            }

            if (data == nullptr || length == 0)
            {
                return RESULT_VOID_FAILUREF("decodeInto: invalid buffer (null or empty)");
            }

            pb_istream_t is = pb_istream_from_buffer(data, length);

            if (!pb_decode(&is, acousea_CommunicationPacket_fields, out))
            {
                return RESULT_VOID_FAILUREF("decodeInto: pb_decode failed: %s", PB_GET_ERROR(&is));
            }
            return RESULT_VOID_SUCCESS();
        }

        // ================================ TO BUFFER ================================
        Result<std::vector<uint8_t>> encode(const acousea_CommunicationPacket& pkt)
        {
            pb_ostream_t sizing = PB_OSTREAM_SIZING;
            if (!pb_encode(&sizing, acousea_CommunicationPacket_fields, &pkt))
            {
                LOG_CLASS_ERROR("encodePacket(SIZE) -> %s", PB_GET_ERROR(&sizing));
                return RESULT_CLASS_FAILUREF(std::vector<uint8_t>, "encodePacket (size): pb_encode failed: %s",
                                             PB_GET_ERROR(&sizing));
            }

            std::vector<uint8_t> buf(sizing.bytes_written);
            pb_ostream_t os = pb_ostream_from_buffer(buf.data(), buf.size());
            if (!pb_encode(&os, acousea_CommunicationPacket_fields, &pkt))
            {
                LOG_CLASS_ERROR("encodePacket(WRITE) -> %s", PB_GET_ERROR(&os));
                return RESULT_CLASS_FAILUREF(std::vector<uint8_t>, "encodePacket (write): pb_encode failed: %s",
                                             PB_GET_ERROR(&os));
            }
            return RESULT_SUCCESS(std::vector<uint8_t>, std::move(buf));
        }


        Result<size_t> encodeInto(const acousea_CommunicationPacket& pkt, uint8_t* buffer, size_t bufferSize)
        {
            if (buffer == nullptr || bufferSize == 0)
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto: invalid buffer (null or empty)");
            }

            // Primer paso: determinar tamaño necesario
            pb_ostream_t sizing = PB_OSTREAM_SIZING;
            if (!pb_encode(&sizing, acousea_CommunicationPacket_fields, &pkt))
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto(SIZE): pb_encode failed: %s", PB_GET_ERROR(&sizing));
            }

            const size_t required = sizing.bytes_written;
            if (required > bufferSize)
            {
                return RESULT_CLASS_FAILUREF(size_t,
                                             "encodeInto: buffer too small (required=%zu, available=%zu)", required,
                                             bufferSize);
            }

            // Segundo paso: codificar realmente
            pb_ostream_t os = pb_ostream_from_buffer(buffer, bufferSize);
            if (!pb_encode(&os, acousea_CommunicationPacket_fields, &pkt))
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto(WRITE): pb_encode failed: %s", PB_GET_ERROR(&os));
            }

            return RESULT_SUCCESS(size_t, os.bytes_written);
        }
    }

    namespace NodeConfiguration
    {
        CLASS_NAME(ProtoUtils::NodeConfiguration)

        // ================================= TO BUFFER ================================

        Result<std::vector<uint8_t>> encode(const acousea_NodeConfiguration& m)
        {
            pb_ostream_t s1 = PB_OSTREAM_SIZING;
            if (!pb_encode(&s1, acousea_NodeConfiguration_fields, &m))
            {
                return RESULT_CLASS_FAILUREF(std::vector<uint8_t>, "encodeProto (size): pb_encode failed: %s",
                                             PB_GET_ERROR(&s1));
            }

            std::vector<uint8_t> buf(s1.bytes_written);
            pb_ostream_t s2 = pb_ostream_from_buffer(buf.data(), buf.size());
            if (!pb_encode(&s2, acousea_NodeConfiguration_fields, &m))
            {
                return RESULT_CLASS_FAILUREF(std::vector<uint8_t>, "encodeProto (write): pb_encode failed: %s",
                                             PB_GET_ERROR(&s2));
            }

            return RESULT_SUCCESS(std::vector<uint8_t>, std::move(buf));
        }

        // ================================= INTO BUFFER ================================

        Result<acousea_NodeConfiguration> decode(const uint8_t* data, const size_t length)
        {
            acousea_NodeConfiguration m = acousea_NodeConfiguration_init_default;

            if (data == nullptr || length == 0)
            {
                return RESULT_CLASS_FAILUREF(acousea_NodeConfiguration, "decodeProto: invalid buffer (null or empty)");
            }

            pb_istream_t is = pb_istream_from_buffer(data, length);

            if (!pb_decode(&is, acousea_NodeConfiguration_fields, &m))
            {
                return RESULT_CLASS_FAILUREF(acousea_NodeConfiguration, "decodeProto: pb_decode failed: %s",
                                             PB_GET_ERROR(&is));
            }

            return RESULT_SUCCESS(acousea_NodeConfiguration, m);
        }

        Result<size_t> encodeInto(const acousea_NodeConfiguration& conf, uint8_t* buffer, size_t bufferSize)
        {
            if (buffer == nullptr || bufferSize == 0)
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto: invalid buffer (null or empty)");
            }

            // Primer paso: obtener tamaño necesario
            pb_ostream_t sizing = PB_OSTREAM_SIZING;
            if (!pb_encode(&sizing, acousea_NodeConfiguration_fields, &conf))
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto(SIZE): pb_encode failed: %s", PB_GET_ERROR(&sizing));
            }

            const size_t required = sizing.bytes_written;
            if (required > bufferSize)
            {
                return RESULT_CLASS_FAILUREF(size_t,
                                             "encodeInto: buffer too small (required=%zu, available=%zu)", required,
                                             bufferSize);
            }

            // Segundo paso: codificar realmente
            pb_ostream_t os = pb_ostream_from_buffer(buffer, bufferSize);
            if (!pb_encode(&os, acousea_NodeConfiguration_fields, &conf))
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto(WRITE): pb_encode failed: %s", PB_GET_ERROR(&os));
            }

            return RESULT_SUCCESS(size_t, os.bytes_written);
        }

        Result<void> decodeInto(const uint8_t* data, const size_t length, acousea_NodeConfiguration* out)
        {
            if (out == nullptr)
            {
                return RESULT_VOID_FAILUREF("decodeInto: destination pointer is null");
            }

            if (data == nullptr || length == 0)
            {
                return RESULT_VOID_FAILUREF("decodeInto: invalid buffer (null or empty)");
            }

            pb_istream_t is = pb_istream_from_buffer(data, length);

            if (!pb_decode(&is, acousea_NodeConfiguration_fields, out))
            {
                return RESULT_VOID_FAILUREF("decodeInto: pb_decode failed: %s", PB_GET_ERROR(&is));
            }

            return RESULT_VOID_SUCCESS();
        }
    }

    namespace ModuleWrapper
    {
        // Codifica un acousea_ModuleWrapper en el buffer proporcionado
        Result<size_t> encodeInto(const acousea_ModuleWrapper& moduleWrapper, uint8_t* buffer, const size_t bufferSize)
        {
            if (buffer == nullptr || bufferSize == 0)
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto: invalid buffer (null or empty)");
            }

            // Primer paso: obtener el tamaño necesario para la codificación
            pb_ostream_t sizing = PB_OSTREAM_SIZING;
            if (!pb_encode(&sizing, acousea_ModuleWrapper_fields, &moduleWrapper))
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto(SIZE): pb_encode failed: %s", PB_GET_ERROR(&sizing));
            }

            const size_t required = sizing.bytes_written;
            if (required > bufferSize)
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto: buffer too small (required=%zu, available=%zu)",
                                             required, bufferSize);
            }

            // Segundo paso: codificar realmente en el buffer proporcionado
            pb_ostream_t os = pb_ostream_from_buffer(buffer, bufferSize);
            if (!pb_encode(&os, acousea_ModuleWrapper_fields, &moduleWrapper))
            {
                return RESULT_CLASS_FAILUREF(size_t, "encodeInto(WRITE): pb_encode failed: %s", PB_GET_ERROR(&os));
            }

            return RESULT_SUCCESS(size_t, os.bytes_written);
        }

        // Decodifica un acousea_ModuleWrapper desde el buffer proporcionado
        Result<void> decodeInto(const uint8_t* data, size_t length, acousea_ModuleWrapper* moduleWrapper)
        {
            if (moduleWrapper == nullptr)
            {
                return RESULT_VOID_FAILUREF("decodeInto: destination pointer is null");
            }

            if (data == nullptr || length == 0)
            {
                return RESULT_VOID_FAILUREF("decodeInto: invalid buffer (null or empty)");
            }

            // Decodifica el buffer en el módulo proporcionado
            pb_istream_t is = pb_istream_from_buffer(data, length);
            if (!pb_decode(&is, acousea_ModuleWrapper_fields, moduleWrapper))
            {
                return RESULT_VOID_FAILUREF("decodeInto: pb_decode failed: %s", PB_GET_ERROR(&is));
            }

            return RESULT_VOID_SUCCESS();
        }
    }
} // ProtoUtils
