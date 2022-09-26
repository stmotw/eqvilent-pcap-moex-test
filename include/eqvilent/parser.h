#pragma once
#include <optional>

namespace eqvilent {
    template<typename ParserType>
    concept PacketParser = requires (ParserType parser, uint8_t* buffer, size_t bufferLength) {
        typename ParserType::packet_type;
        { ParserType::parse(buffer, bufferLength) } -> std::same_as<std::optional<typename ParserType::packet_type>>;
    };
}
