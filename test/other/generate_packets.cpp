#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>


#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"
#include "nodeDevice.pb.h"

// Compile with:
//  g++ D:\Documents\GitHub\Acousea\acousea-infrastructure-mkr\test\other\generate_packets.cpp
//    D:\Documents\GitHub\Acousea\acousea-infrastructure-mkr\.pio\libdeps\mkrgsm1400-test\Nanopb\pb_common.c
//    D:\Documents\GitHub\Acousea\acousea-infrastructure-mkr\.pio\libdeps\mkrgsm1400-test\Nanopb\pb_encode.c
//    D:\Documents\GitHub\Acousea\acousea-infrastructure-mkr\.pio\libdeps\mkrgsm1400-test\Nanopb\pb_decode.c
//    D:\Documents\GitHub\Acousea\acousea-infrastructure-mkr\lib\NodeDevice\bindings\nodeDevice.pb.c
//    -I D:\Documents\GitHub\Acousea\acousea-infrastructure-mkr\lib\NodeDevice\bindings
//    -I D:\Documents\GitHub\Acousea\acousea-infrastructure-mkr\.pio\libdeps\mkrgsm1400-test\Nanopb
//    -o generate_packets

// ==========================================================
//  Helper: encodePacket / decodePacket (independientes de Router)
// ==========================================================

std::vector<uint8_t> encodePacket(const acousea_CommunicationPacket &pkt)
{
    uint8_t buffer[acousea_CommunicationPacket_size] = {0};
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, acousea_CommunicationPacket_fields, &pkt))
    {
        throw std::runtime_error(std::string("Encoding failed: ") + PB_GET_ERROR(&stream));
    }

    return std::vector<uint8_t>(buffer, buffer + stream.bytes_written);
}

acousea_CommunicationPacket decodePacket(const std::vector<uint8_t> &raw)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(raw.data(), raw.size());

    if (!pb_decode(&stream, acousea_CommunicationPacket_fields, &pkt))
    {
        throw std::runtime_error(std::string("Decoding failed: ") + PB_GET_ERROR(&stream));
    }

    return pkt;
}

// ==========================================================
//  MAIN
// ==========================================================

// ==========================================================
//  Main with added SOF and Length byte
// ==========================================================

int main()
{
    using namespace std;

    // -------------- COMMAND PACKET --------------
    acousea_CommunicationPacket commandPkt = acousea_CommunicationPacket_init_zero;
    commandPkt.packetId = 1;
    commandPkt.has_routing = true;
    commandPkt.routing.sender = 2;
    commandPkt.routing.receiver = 0;
    commandPkt.routing.ttl = 1;

    // Body -> Command
    commandPkt.which_body = 3; // "command"
    commandPkt.body.command.which_command = 2; // requestedConfiguration
    commandPkt.body.command.command.requestedConfiguration.requestedModules_count = 2;
    commandPkt.body.command.command.requestedConfiguration.requestedModules[0] = acousea_ModuleCode_ICLISTEN_STATUS;
    commandPkt.body.command.command.requestedConfiguration.requestedModules[1] =
        acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG;

    // Encode
    std::vector<uint8_t> bytesCmd;
    try
    {
        bytesCmd = encodePacket(commandPkt);
    }
    catch (const std::exception &e)
    {
        cerr << "Error encoding command packet: " << e.what() << endl;
        return 1;
    }

    // Add SOF (0x2A) and Length (1 byte for payload length)
    uint8_t SOF = 0x2A;  // Start of Frame byte
    uint8_t payloadLength = bytesCmd.size();  // Length of the payload (after SOF and length)

    // Insert SOF and Length byte at the beginning of the byte vector
    bytesCmd.insert(bytesCmd.begin(), payloadLength);  // Insert length first
    bytesCmd.insert(bytesCmd.begin(), SOF);  // Insert SOF byte

    // Now we can print the result to stdout as hex string
    ostringstream cmdHex;
    cmdHex << "printf \"";
    for (uint8_t b : bytesCmd)
    {
        cmdHex << "\\x" << hex << uppercase << setw(2) << setfill('0') << (int)b;
    }
    cmdHex << "\" > /dev/ttyS2";

    cout << "\n--- COMMAND PACKET ---" << endl;
    cout << cmdHex.str() << endl;


    // -------------- ERROR PACKET --------------
    acousea_CommunicationPacket errorPkt = acousea_CommunicationPacket_init_zero;
    errorPkt.packetId = 2;
    errorPkt.has_routing = true;
    errorPkt.routing.sender = 0;
    errorPkt.routing.receiver = 2;
    errorPkt.routing.ttl = 1;

    // Body -> Error
    errorPkt.which_body = 6; // "error"
    strcpy(errorPkt.body.error.errorMessage, "Invalid command receivedabc");

    // Encode
    std::vector<uint8_t> bytesErr;
    try
    {
        bytesErr = encodePacket(errorPkt);
    }
    catch (const std::exception &e)
    {
        cerr << "Error encoding error packet: " << e.what() << endl;
        return 1;
    }

    // Add SOF (0x2A) and Length (1 byte for payload length)
    SOF = 0x2A;
    payloadLength = bytesErr.size();  // Length of the payload (after SOF and length)

    // Insert SOF and Length byte at the beginning of the byte vector
    bytesErr.insert(bytesErr.begin(), payloadLength);  // Insert length first
    bytesErr.insert(bytesErr.begin(), SOF);  // Insert SOF byte

    // Now we can print the result to stdout as hex string
    ostringstream errHex;
    errHex << "printf \"";
    for (uint8_t b : bytesErr)
    {
        errHex << "\\x" << hex << uppercase << setw(2) << setfill('0') << (int)b;
    }
    errHex << "\" > /dev/ttyS2";

    cout << "\n--- ERROR PACKET ---" << endl;
    cout << errHex.str() << endl;

    return 0;
}