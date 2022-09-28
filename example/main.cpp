#include <cstdio>
#include <cstdlib>
#include "eqvilent/pcap.h"
#include "eqvilent/packets.h"
#include "eqvilent/spectra_simba.h"

using eqvilent::network::pcap::PCAPReader;
using eqvilent::network::packets::UDPParser;
using eqvilent::moex::spectra_simba::SpectraSimbaParser;

int main() {
    auto pcapReader = PCAPReader<UDPParser<SpectraSimbaParser>>();

    size_t no_of_packets = 0;
    for (auto packet = pcapReader.next(); packet.has_value(); packet = pcapReader.next()) {
        ++no_of_packets;
    }
    printf("Read %ld packets\n", no_of_packets);

    return 0;
}
