#pragma once
#include <cstdint>

namespace eqvilent::network::packets {
    struct EthernetIIHeader {
        uint8_t destination[6];
        uint8_t source[6];
        uint8_t type[2];
    };

    // simplification:
    // - grouping up some fields for easy pasrsing.
    // - assume that IHL always equals 5, so there are no additional IPv4 headers
    struct IPv4Header {
        uint8_t versionAndIHL;
        uint8_t DCSPAndECN;
        uint8_t totalLength[2];
        uint8_t identification[2];
        uint8_t flagsAndFragmentOffset[2];
        uint8_t timeToLive;
        uint8_t protocol;
        uint8_t headerChecksum[2];
        uint8_t sourceIpAddress[4];
        uint8_t destinationIpAddress[4];
    };

    struct UDPHeader {
        uint8_t sourcePort[2];
        uint8_t destinationPort[2];
        uint8_t length[2];
        uint8_t checksum[2];
    };

    template<PacketParser Parser>
    class UDPParser {
    public:
        typedef typename Parser::packet_type packet_type;

        static std::optional<packet_type> parse(uint8_t* buffer, size_t bufferLength) {
            if (bufferLength < sizeof(EthernetIIHeader)) {
                return {};
            }
            [[maybe_unused]] EthernetIIHeader* ethernetHeader = reinterpret_cast<EthernetIIHeader*>(buffer);
            buffer += sizeof(EthernetIIHeader);
            bufferLength -= sizeof(EthernetIIHeader);
            
            if (bufferLength < sizeof(IPv4Header)) {
                return {};
            }
            [[maybe_unused]] IPv4Header* ipv4Header = reinterpret_cast<IPv4Header*>(buffer);
            buffer += sizeof(IPv4Header);
            bufferLength -= sizeof(IPv4Header);

            if (bufferLength < sizeof(UDPHeader)) {
                return {};
            }
            [[maybe_unused]] UDPHeader* udpHeader = reinterpret_cast<UDPHeader*>(buffer);
            buffer += sizeof(UDPHeader);
            bufferLength -= sizeof(UDPHeader);

            if (size_t udpDataLength = (size_t(udpHeader->length[0]) << 8) + size_t(udpHeader->length[1]);
                udpDataLength != sizeof(UDPHeader) + bufferLength) {
                return {};
            }
            
            return Parser::parse(buffer, bufferLength);
        }
    };
}
