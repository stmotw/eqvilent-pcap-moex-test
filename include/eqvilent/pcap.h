#pragma once
#include <cstdint>
#include <optional>
#include "eqvilent/parser.h"

// pcap parser. See format description at https://www.tcpdump.org/manpages/pcap-savefile.5.txt
namespace eqvilent::network::pcap {
    struct FileHeader {
        uint32_t magicNumber;
        uint16_t majorVersion;
        uint16_t minorVersion;
        uint32_t timeZoneOffset; // always 0
        uint32_t timestampAccuracy; // always 0
        uint32_t snapshotLength;
        uint32_t linkLayerHeader;
    } __attribute__((packed));

    struct PacketRecordHeader {
        uint32_t timestampSeconds;
        uint32_t timestampSubseconds;
        uint32_t capturedPacketLength;
        uint32_t originalPacketLength;
    } __attribute__((packed));

    template<PacketParser PCAPBodyParser>
    class PCAPReader {
        uint8_t* buffer;
        PCAPBodyParser parser;

    public:
        // simplification, we're not checking
        // - magic number, it's microseconds for moex pcap dumps
        // - version, it's always 2.4
        // - timeZoneOffset and timestampAccuracy to be equal zero, indicating no data corruption
        // - linkLayerHeader info
        PCAPReader() 
            : buffer(nullptr)
            , parser{}
        {
            FileHeader fileHeader;
            if (fread(reinterpret_cast<uint8_t*>(&fileHeader), 1, sizeof(FileHeader), stdin)) {
                buffer = reinterpret_cast<uint8_t*>(malloc(fileHeader.snapshotLength));
            }
        }

        ~PCAPReader() {
            free(buffer);
        }

        std::optional<typename PCAPBodyParser::packet_type> next() {
            PacketRecordHeader packetRecord;

            // cannot read next packet header
            if (!fread(reinterpret_cast<uint8_t*>(&packetRecord), 1, sizeof(pcap::PacketRecordHeader), stdin)) {
                return std::nullopt;
            }
            // simplification, moex dump does not have large packets
            if (packetRecord.capturedPacketLength != packetRecord.originalPacketLength) {
                return std::nullopt;
            }

            if (!fread(buffer, 1, packetRecord.capturedPacketLength, stdin)) {
                return std::nullopt;
            }
            return parser.parse(reinterpret_cast<uint8_t *>(buffer), packetRecord.capturedPacketLength);
        }
    };
}
