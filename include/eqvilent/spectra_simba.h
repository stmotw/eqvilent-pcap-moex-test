#pragma once
#include <cstdint>

template <typename T>
void binPrint(const T* data, const char prefix[]) {
    printf("%s: ", prefix);

    const uint8_t* uint8_t_ptr = reinterpret_cast<const uint8_t*>(data);
    size_t kk = sizeof(T);
    while(kk--)
    {
        uint8_t c = *uint8_t_ptr++;
        printf("%.2x ", c);
    }
    printf("\n");
}

void binPrint(const uint8_t* buffer, size_t bufferLength) {
    printf("buf: ");

    const uint8_t* uint8_t_ptr = buffer;
    size_t kk = bufferLength;
    while(kk--)
    {
        uint8_t c = *uint8_t_ptr++;
        printf("%.2x ", c);
    }
    printf("\n");
}

namespace eqvilent::moex::spectra_simba {
    struct MarketDataPacketHeader {
        uint32_t msgSeqNum;
        uint16_t msgSize;
        uint16_t msgFlags;
        uint64_t sendingTime;
    };

    struct IncrementalPacketHeader {
        uint64_t transactTime;
        uint32_t exchangeTradingSessionId;
    };

    struct SBEHeader {
        uint16_t blockLength;
        uint16_t templateId;
        uint16_t schemaId;
        uint16_t version;
    };

    struct OrderUpdate {

    };

    struct OrderExecution {

    };

    struct OrderBookSnapshot {

    };
}
