#ifndef ACOUSEA_INFRASTRUCTURE_MKR_PACKETUTILS_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_PACKETUTILS_HPP

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <pb_encode.h>
#include <pb_decode.h>
#include "bindings/nodeDevice.pb.h"


namespace PacketUtils
{
    // ======================================================================
    // Helpers nanopb
    // ======================================================================
    static std::vector<uint8_t> encodePacketTest(const acousea_CommunicationPacket& pkt)
    {
        pb_ostream_t sizing = PB_OSTREAM_SIZING;
        if (!pb_encode(&sizing, acousea_CommunicationPacket_fields, &pkt))
            throw std::runtime_error(std::string("encodePacketTest sizing failed: ") + PB_GET_ERROR(&sizing));

        std::vector<uint8_t> buf(sizing.bytes_written);
        pb_ostream_t os = pb_ostream_from_buffer(buf.data(), buf.size());
        if (!pb_encode(&os, acousea_CommunicationPacket_fields, &pkt))
            throw std::runtime_error(std::string("encodePacketTest write failed: ") + PB_GET_ERROR(&os));

        return buf;
    }

    static acousea_CommunicationPacket decodePacketTest(const std::vector<uint8_t>& raw)
    {
        acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
        pb_istream_t is = pb_istream_from_buffer(raw.data(), raw.size());
        if (!pb_decode(&is, acousea_CommunicationPacket_fields, &pkt))
            throw std::runtime_error(std::string("decodePacketTest failed: ") + PB_GET_ERROR(&is));
        return pkt;
    }

    static acousea_CommunicationPacket makeRoutedPacket(uint8_t sender, uint8_t receiver)
    {
        acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
        pkt.has_routing = true;
        pkt.routing = acousea_RoutingChunk_init_default;
        pkt.routing.sender = sender;
        pkt.routing.receiver = receiver;

        // Asignamos un body "command.setConfiguration" vacío válido
        pkt.which_body = acousea_CommunicationPacket_command_tag;
        pkt.body.command = acousea_CommandBody_init_default;
        pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
        pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

        return pkt;
    }

    // Crea un paquete válido base para setConfiguration
    static acousea_CommunicationPacket makeBaseSetConfigPacket() {
        acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
        pkt.has_routing = true;
        pkt.routing.sender = 1;
        pkt.routing.receiver = 255;
        pkt.which_body = acousea_CommunicationPacket_command_tag;
        pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
        pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;
        return pkt;
    }


    static acousea_CommunicationPacket makeBaseResponsePacket(uint8_t responseType)
    {
        acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
        pkt.has_routing = true;
        pkt.routing.sender = 1;
        pkt.routing.receiver = 255;
        pkt.which_body = acousea_CommunicationPacket_response_tag;
        pkt.body.response.which_response = responseType;
        return pkt;
    }


}

#endif //ACOUSEA_INFRASTRUCTURE_MKR_PACKETUTILS_HPP
