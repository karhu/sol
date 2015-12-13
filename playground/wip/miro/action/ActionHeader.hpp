#pragma once

namespace miro { namespace action {

    struct HeaderMeta {
        HeaderMeta() {}
        HeaderMeta(uint16_t timestamp, uint8_t user = 0) : timestamp(timestamp), user(user) {}

        uint16_t timestamp = 0;
        uint8_t user = 0;
    };

    struct HeaderData {
        HeaderData() {}
        HeaderData(ActionType type, HeaderMeta meta)
            : type(type), user(meta.user), timestamp(meta.timestamp)
        {}
        HeaderData(ActionType type, uint8_t user = 0, uint16_t timestamp = 0)
            : type(type), user(user), timestamp(timestamp) {}

        ActionType type = ActionType::Unknown;
        uint8_t user = 0;
        uint16_t timestamp = 0; // timestamp in 10ms steps, rolls over every 10min
    };

    struct ActionHeader {
        ActionHeader() {}
        HeaderData meta;
        RelativeMemoryRange16 memory;
    };

}}

