#include <Arduino.h>
#include <BitPacker.h>
#include <config.h>

class PacketBuilder {
public:
    static bool buildDrifterDataPacket(uint8_t *buffer, size_t bufferSize,
                                       data_info_t data, size_t *buffer_bytes_sizep) {
        uint32_t arg;
        size_t bit_pos = 0;
        size_t length_bit_pos;
        size_t bitstr_sz = bufferSize << 3;

        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)DUE_MSG, (size_t)4);
        length_bit_pos = bit_pos;
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)0, (size_t)12);
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, DRIFTER_DATA_MSG, (size_t)8);
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, data.epoch, (size_t)32);

        arg = (uint32_t)round(((double)data.latitude_deg + 90.0) * 1.0E6);
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, arg, (size_t)28);
        arg = (uint32_t)round(((double)data.longitude_deg + 180.0) * 1.0E6);
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, arg, (size_t)29);

        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, data.battery_soc, (size_t)7);
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, data.includes_vr2c_data, (size_t)1);

        if (data.includes_vr2c_data) {
            tagSummary_t *summary = &data.tagSummary;

            bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)summary->num_tag_maps, (size_t)4);
            for (int i = 0; i < int(summary->num_tag_maps); i++) {
                bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)summary->tagMap[i], (size_t)14);
            }

            bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)summary->detectedTags, (size_t)4);
            for (int i = 0; i < int(summary->detectedTags); i++) {
                bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)summary->tagID[i], (size_t)14);
            }

            for (int i = 0; i < int(summary->detectedTags); i++) {
                bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)summary->tagMapIndex[i], (size_t)4);
            }

            for (int i = 0; i < int(summary->detectedTags); i++) {
                bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)summary->detections[i], (size_t)16);
            }

            bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)summary->overflow, (size_t)1);
        }

        BitPacker::pack(buffer, length_bit_pos, bitstr_sz, (uint32_t)bit_pos, (size_t)12);
        *buffer_bytes_sizep = bit_pos / 8 + ((bit_pos % 8) ? 1 : 0);

        return bit_pos <= bitstr_sz;
    }

    static bool parseDrifterDataPacket(uint8_t *buffer, data_info_t &data) {
        uint32_t arg;
        uint8_t msgType;
        size_t bit_pos = 0;
        size_t bitstr_sz = 16;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)4);
        msgType = (uint8_t)arg;
        if (msgType != DUE_MSG) return false;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)12);
        bitstr_sz = (uint16_t)arg;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)8);
        if ((uint8_t)arg != DRIFTER_DATA_MSG) return false;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)32);
        data.epoch = (time_t)arg;
        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, 28);
        data.latitude_deg = (float)(((double)arg) * 1.0E-6 - 90.0);

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, 29);
        data.longitude_deg = (float)(((double)arg) * 1.0E-6 - 180.0);

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)7);
        data.battery_soc = (uint8_t)arg;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)1);
        data.includes_vr2c_data = (arg) ? true : false;

        if (data.includes_vr2c_data) {
            tagSummary_t *summary = &data.tagSummary;
            memset(summary, 0, sizeof(tagSummary_t));

            bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)4);
            summary->num_tag_maps = (uint8_t)arg;

            for (int i = 0; i < int(summary->num_tag_maps); i++) {
                bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)14);
                summary->tagMap[i] = (uint16_t)arg;
            }

            bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)4);
            summary->detectedTags = (uint8_t)arg;

            for (int i = 0; i < int(summary->detectedTags); i++) {
                bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)14);
                summary->tagID[i] = (uint16_t)arg;
            }

            for (int i = 0; i < int(summary->detectedTags); i++) {
                bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)4);
                summary->tagMapIndex[i] = (uint8_t)arg;
            }

            for (int i = 0; i < int(summary->detectedTags); i++) {
                bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)16);
                summary->detections[i] = (uint16_t)arg;
            }

            bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)1);
            summary->overflow = arg ? true : false;
        }

        return bit_pos == bitstr_sz;
    }

    static bool buildDrifterSetModePacket(uint8_t *buffer, size_t bufferSize,
                                          mode_chg_t mode_cmd, size_t *buffer_bytes_sizep) {
        uint32_t arg;
        size_t bit_pos = 0;
        size_t length_bit_pos;
        size_t bitstr_sz = bufferSize << 3;

        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)DUE_MSG, (size_t)4);
        length_bit_pos = bit_pos;
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)0, (size_t)12);
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, DRIFTER_SETMODE_MSG, (size_t)8);
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, (uint32_t)mode_cmd.mode, (size_t)2);

        arg = mode_cmd.sbd_reporting_period_sec / 60;
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, arg, (size_t)11);

        arg = mode_cmd.lora_comm_period_sec / 60;
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, arg, (size_t)11);

        arg = mode_cmd.gnss_logging_period_sec / 60;
        bit_pos = BitPacker::pack(buffer, bit_pos, bitstr_sz, arg, (size_t)11);

        BitPacker::pack(buffer, length_bit_pos, bitstr_sz, (uint32_t)bit_pos, (size_t)12);
        *buffer_bytes_sizep = bit_pos / 8 + ((bit_pos % 8) ? 1 : 0);

        return bit_pos <= bitstr_sz;
    }

    static bool parseDrifterSetModePacket(uint8_t *buffer, mode_chg_t *mode_cmd) {
        uint32_t arg;
        uint8_t msgType;
        size_t bit_pos = 0;
        size_t bitstr_sz = 16;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)4);
        msgType = (uint8_t)arg;
        if (msgType != DUE_MSG) return false;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)12);
        bitstr_sz = (uint16_t)arg;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)8);
        if ((uint8_t)arg != DRIFTER_SETMODE_MSG) return false;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)2);
        mode_cmd->mode = (drifter_mode_t)arg;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)11);
        mode_cmd->sbd_reporting_period_sec = arg * 60;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)11);
        mode_cmd->lora_comm_period_sec = arg * 60;

        bit_pos = BitPacker::unpack(buffer, bit_pos, bitstr_sz, &arg, (size_t)11);
        mode_cmd->gnss_logging_period_sec = arg * 60;

        return bit_pos == bitstr_sz;
    }

    static uint8_t getPacketType(uint8_t *cmd) {
        uint32_t arg;
        BitPacker::unpack(cmd, 0, 16, (uint32_t *)&arg, 4);
        return (uint8_t)arg;
    }

    static uint8_t getPacketSubType(uint8_t *cmd) {
        uint32_t arg;
        size_t bit_pos;
        uint16_t bitstr_sz;

        bit_pos = BitPacker::unpack(cmd, 4, 16, &arg, (size_t)12);
        bitstr_sz = (uint16_t)arg;
        BitPacker::unpack(cmd, bit_pos, bitstr_sz, (uint32_t *)&arg, 8);
        return (uint8_t)arg;
    }

    static size_t getPacketSizeInBits(uint8_t *cmd) {
        uint32_t arg;
        BitPacker::unpack(cmd, 4, 16, (uint32_t *)&arg, 12);
        return (size_t)arg;
    }

    static size_t getPacketSizeInBytes(uint8_t *cmd) {
        uint32_t arg;
        size_t numBytes;
        BitPacker::unpack(cmd, 4, 16, (uint32_t *)&arg, 12);
        numBytes = (size_t)arg >> 3;
        numBytes += (size_t)((arg % 8) ? 1 : 0);
        return numBytes;
    }
};
