#include <cstdio>
#include <cstdlib>
#include "eqvilent/pcap.h"
#include "eqvilent/packets.h"
#include "eqvilent/spectra_simba.h"

using eqvilent::network::pcap::PCAPReader;
using eqvilent::network::packets::UDPParser;

class DumbParser {
public:
    typedef uint8_t packet_type;

    static std::optional<packet_type> parse(uint8_t* buffer, size_t bufferLength) {
        return { 0 };
    }
};

int main() {
    auto pcapStdinReader = PCAPReader<UDPParser<DumbParser>>();

    size_t no_of_packets = 0;
    for (auto packet = pcapStdinReader.next(); packet.has_value(); packet = pcapStdinReader.next()) {
        ++no_of_packets;
    }
    printf("Read %ld packets\n", no_of_packets);

    return 0;
}
