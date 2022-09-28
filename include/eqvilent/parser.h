#pragma once
#include <concepts>
#include <optional>

namespace eqvilent {
    template<typename ParserType>
    concept PacketParser = requires (ParserType parser, uint8_t* buffer, size_t bufferLength) {
        requires std::default_initializable<ParserType>;
        typename ParserType::packet_type;
        { parser.parse(buffer, bufferLength) } -> std::same_as<std::optional<typename ParserType::packet_type>>;
    };
}
