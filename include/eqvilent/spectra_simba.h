#pragma once
#include <cstdint>
#include <type_traits>
#include <variant>

template <typename Enum>
constexpr auto toUnderlying(Enum e) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}

template <typename T>
void binPrint(const T* data, const char prefix[]) {
    printf("%s: ", prefix);

    const uint8_t* uint8_t_ptr = reinterpret_cast<const uint8_t*>(data);
    size_t bytes_left = sizeof(T);
    while(bytes_left--)
    {
        uint8_t c = *uint8_t_ptr++;
        printf("%.2x ", c);
    }
    printf("\n");
}

void binPrint(const uint8_t* buffer, size_t bufferLength) {
    printf("buf: ");

    const uint8_t* uint8_t_ptr = buffer;
    size_t bytes_left = bufferLength;
    while(bytes_left--)
    {
        uint8_t c = *uint8_t_ptr++;
        printf("%.2x ", c);
    }
    printf("\n");
}

namespace eqvilent::moex::spectra_simba {
    typedef uint16_t MsgFlags;
    typedef uint16_t TemplateId;

    enum class MsgFlagsEnum : MsgFlags {
        LAST_FRAGMENT = 1 << 0,
        START_OF_SNAPSHOT = 1 << 1,
        END_OF_SNAPSHOT = 1 << 2,
        INCREMENTAL_PACKET = 1 << 3,
        POSS_DUP_FLAG = 1 << 4,
    };

    enum class TemplateIdEnum : TemplateId {
        // packet with variable body length inside incremental message
        BEST_PRICES = 3,

        ORDER_UPDATE = 5,
        ORDER_EXECUTION = 6,
        ORDER_BOOK_SNAPSHOT = 7,
    };

    struct MarketDataPacketHeader {
        uint32_t msgSeqNum;
        uint16_t msgSize;
        MsgFlags msgFlags;
        uint64_t sendingTime;
    } __attribute__((packed));

    struct IncrementalPacketHeader {
        uint64_t transactTime;
        uint32_t exchangeTradingSessionId;
    } __attribute__((packed));

    struct SBEHeader {
        uint16_t blockLength;
        TemplateId templateId;
        uint16_t schemaId;
        uint16_t version;
    } __attribute__((packed));

    struct GroupSize {
        uint16_t blockLength;
        uint8_t numInGroup;
    } __attribute__((packed));

    struct Decimal5 {
        static const int8_t exponent = -5;
        int64_t mantissa;
    } __attribute__((packed));

    // Empty struct to indicate that packet was parsed, but we don't want to read it's contents
    struct IgnoredPacket {
    } __attribute__((packed));

    struct OrderUpdate {
        int64_t mdEntryID;
        Decimal5 mdEntryPx;
        int64_t mdEntrySize;
        uint64_t mdFlags;
        int32_t securityId;
        uint32_t rptSeq;
        uint8_t mdUpdateAction;
        char mdEntryType;
    } __attribute__((packed));

    struct OrderExecution {
        int64_t mdEntryID;
        Decimal5 mdEntryPx;
        int64_t mdEntrySize;
        Decimal5 lastPx;
        int64_t lastQty;
        int64_t tradeId;
        uint64_t mdFlags;
        int32_t securityId;
        uint32_t rptSeq;
        uint8_t mdUpdateAction;
        char mdEntryType;
    } __attribute__((packed));

    struct OrderBookSnapshot {
        int32_t securityId;
        uint32_t lastMsgSeqNumProcessed;
        uint32_t rptSeq;
        uint32_t exchangeTradingSessionID;
        GroupSize noMDEntries;
    } __attribute__((packed));

    struct MDEntry {
        int64_t mdEntryID;
        uint64_t transactTime;
        Decimal5 mdEntryPx;
        int64_t mdEntrySize;
        int64_t tradeId;
        uint64_t mdFlags;
        char mdEntryType;
    } __attribute__((packed));

    class SpectraSimbaParser {
    public:
        // TODO: 
        // - Return actual data from this reader. For now type readers just print the data.
        // - Introduce internal state for:
        //   - Collecting OrderBookSnapshot throughout multiple messages
        //   - Returning OrderUpdate and OrderExecution messages bundled in incremental packet one by one
        // typedef std::variant<IgnoredPacket, OrderUpdate, OrderExecution, OrderBookSnapshot> packet_type;
        typedef std::variant<IgnoredPacket> packet_type;

        std::optional<packet_type> parse(uint8_t* buffer, size_t bufferLength) {
            MarketDataPacketHeader* marketDataPacketHeader = reinterpret_cast<MarketDataPacketHeader*>(buffer);
            buffer += sizeof(MarketDataPacketHeader);
            bufferLength -= sizeof(MarketDataPacketHeader);

            if (marketDataPacketHeader->msgFlags & toUnderlying(MsgFlagsEnum::INCREMENTAL_PACKET)) {
                return parseIncrementalPacket(buffer, bufferLength);
            } else {
                return parseSnapshotPacket(buffer, bufferLength);
            }
        }

    private:
        std::optional<packet_type> parseIncrementalPacket(uint8_t* buffer, size_t bufferLength) {
            [[maybe_unused]] IncrementalPacketHeader* incrementalPacketHeader =
                reinterpret_cast<IncrementalPacketHeader*>(buffer);
            buffer += sizeof(IncrementalPacketHeader);
            bufferLength -= sizeof(IncrementalPacketHeader);

            while (bufferLength > 0) {
                std::optional<size_t> bytes_read = parseSBEMessage(buffer, bufferLength);
                if (!bytes_read.has_value()) {
                    return std::nullopt;
                }
                buffer += bytes_read.value();
                bufferLength -= bytes_read.value();
            }

            return { IgnoredPacket{} };
        }

        std::optional<packet_type> parseSnapshotPacket(uint8_t* buffer, size_t bufferLength) {
            while (bufferLength > 0) {
                std::optional<size_t> bytes_read = parseSBEMessage(buffer, bufferLength);
                if (!bytes_read.has_value()) {
                    return std::nullopt;
                }
                buffer += bytes_read.value();
                bufferLength -= bytes_read.value();
            }

            return { IgnoredPacket{} };
        }

        std::optional<size_t> parseSBEMessage(uint8_t* buffer, size_t bufferLength) {
            SBEHeader* sbeHeader = reinterpret_cast<SBEHeader*>(buffer);
            buffer += sizeof(SBEHeader);
            bufferLength -= sizeof(SBEHeader);

            std::optional<size_t> number_of_bytes_read =
                parseSBEMessageBody(buffer, bufferLength, sbeHeader->templateId);
            if (number_of_bytes_read.has_value()) {
                return { sizeof(SBEHeader) + number_of_bytes_read.value() };
            }
            return std::nullopt;
        }

        std::optional<size_t> parseSBEMessageBody(
            uint8_t* buffer,
            size_t bufferLength,
            TemplateId templateId
        ) {
            switch (templateId) {
                case toUnderlying(TemplateIdEnum::BEST_PRICES):
                    return parseBestPrices(buffer, bufferLength);
                case toUnderlying(TemplateIdEnum::ORDER_UPDATE):
                    return parseOrderUpdate(buffer, bufferLength);
                case toUnderlying(TemplateIdEnum::ORDER_EXECUTION):
                    return parseOrderExecution(buffer, bufferLength);
                case toUnderlying(TemplateIdEnum::ORDER_BOOK_SNAPSHOT):
                    return parseOrderBookSnapshot(buffer, bufferLength);
                default:
                    return { bufferLength };
            }
        }

        std::optional<size_t> parseBestPrices(uint8_t* buffer, size_t bufferLength) {
            GroupSize* groupSize = reinterpret_cast<GroupSize*>(buffer);
            buffer += sizeof(groupSize);
            bufferLength -= sizeof(groupSize);
            
            if (bufferLength < groupSize->numInGroup * groupSize->blockLength) {
                return std::nullopt;
            }
            return { sizeof(GroupSize) + groupSize->blockLength * groupSize->numInGroup };
        }

        std::optional<size_t> parseOrderUpdate(uint8_t* buffer, size_t bufferLength) {
            OrderUpdate* orderUpdate = reinterpret_cast<OrderUpdate*>(buffer);

            printf(
                "OrderUpdate\n"
                "  MDEntryID: %ld\n"
                "  MDEntryPx: %lde%d\n"
                "  MDEntrySize: %ld\n"
                "  MDFlags: %.16lx\n"
                "  SecurityID: %d\n"
                "  RptSeq: %u\n"
                "  MDUpdateAction: %.2x\n"
                "  MDEntryType: %c\n\n",
                orderUpdate->mdEntryID,
                orderUpdate->mdEntryPx.mantissa,
                orderUpdate->mdEntryPx.exponent,
                orderUpdate->mdEntrySize,
                orderUpdate->mdFlags,
                orderUpdate->securityId,
                orderUpdate->rptSeq,
                orderUpdate->mdUpdateAction,
                orderUpdate->mdEntryType);

            return { sizeof(OrderUpdate) };
        }

        std::optional<size_t> parseOrderExecution(uint8_t* buffer, size_t bufferLength) {
            OrderExecution* orderExeciton = reinterpret_cast<OrderExecution*>(buffer);

            printf(
                "OrderExecution\n"
                "  MDEntryID: %ld\n"
                "  MDEntryPx: %lde%d\n"
                "  MDEntrySize: %ld\n"
                "  LastPx: %lde%d\n"
                "  LastQty: %ld\n"
                "  TradeID: %ld\n"
                "  MDFlags: %.16lx\n"
                "  SecurityID: %d\n"
                "  RptSeq: %u\n"
                "  MDUpdateAction: %.2x\n"
                "  MDEntryType: %c\n\n",
                orderExeciton->mdEntryID,
                orderExeciton->mdEntryPx.mantissa,
                orderExeciton->mdEntryPx.exponent,
                orderExeciton->mdEntrySize,
                orderExeciton->lastPx.mantissa,
                orderExeciton->lastPx.exponent,
                orderExeciton->lastQty,
                orderExeciton->tradeId,
                orderExeciton->mdFlags,
                orderExeciton->securityId,
                orderExeciton->rptSeq,
                orderExeciton->mdUpdateAction,
                orderExeciton->mdEntryType);

            return { sizeof(OrderExecution) };
        }

        std::optional<size_t> parseOrderBookSnapshot(uint8_t* buffer, size_t bufferLength) {
            OrderBookSnapshot* orderBookSnapshot = reinterpret_cast<OrderBookSnapshot*>(buffer);
            buffer += sizeof(OrderBookSnapshot);
            bufferLength -= sizeof(OrderBookSnapshot);
            if (bufferLength != orderBookSnapshot->noMDEntries.numInGroup * orderBookSnapshot->noMDEntries.blockLength) {
                return std::nullopt;
            }

            printf(
                "OrderBookSnapshot\n"
                "  SecurityID: %d\n"
                "  LastMsgSeqNumProcessed: %u\n"
                "  RptSeq: %u\n"
                "  ExchangeTradingSessionID: %u\n"
                "  NoMDEntries: %u items * %u bytes\n"
                "\n",
                orderBookSnapshot->securityId,
                orderBookSnapshot->lastMsgSeqNumProcessed,
                orderBookSnapshot->rptSeq,
                orderBookSnapshot->exchangeTradingSessionID,
                orderBookSnapshot->noMDEntries.numInGroup,
                orderBookSnapshot->noMDEntries.blockLength);

            for (size_t i = 0; i < orderBookSnapshot->noMDEntries.numInGroup; ++i) {
                if (bufferLength < sizeof(MDEntry)) {
                    return std::nullopt;
                }
                MDEntry* mdEntry = reinterpret_cast<MDEntry*>(buffer);
                buffer += sizeof(MDEntry);
                bufferLength -= sizeof(MDEntry);

                printf(
                    "  item #%lu\n"
                    "    MDEntryID: %ld\n"
                    "    TransactTime: %lu\n"
                    "    MDEntryPx: %lde%d\n"
                    "    MDEntrySize: %ld\n"
                    "    TradeID: %ld\n"
                    "    MDFlags: %.16lx\n"
                    "    MDEntryType: %c\n\n",
                    i,
                    mdEntry->mdEntryID,
                    mdEntry->transactTime,
                    mdEntry->mdEntryPx.mantissa,
                    mdEntry->mdEntryPx.exponent,
                    mdEntry->mdEntrySize,
                    mdEntry->tradeId,
                    mdEntry->mdFlags,
                    mdEntry->mdEntryType);
            }
            
            return {
                sizeof(OrderBookSnapshot)
                + orderBookSnapshot->noMDEntries.numInGroup * orderBookSnapshot->noMDEntries.blockLength
            };
        }
    };
}
