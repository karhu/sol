#pragma once

#include "vec2.hpp"

#include "ActionReference.hpp"

namespace miro { namespace action {

    // Stroke Action //

    struct StrokeActionData {
    public:
        static constexpr ActionType Type = ActionType::Stroke;
    public:
        vec2f position;   // position in the current view frame
        float pressure;   // pressure in [0,1]
        uint8_t button;   // a button, finger, tip id
        uint8_t type = 0; // 0 for update, 1 for begin, 2 for end
    };

    struct StrokeActionRef : public ActionReference<StrokeActionData>
    {
        StrokeActionRef() {}
        StrokeActionRef(ActionBuffer& b, uint16_t ai) : ActionReference(b,ai) {}

        vec2f position() { return data().position; }
        float  pressure() { return data().pressure; }
        uint8_t button() { return data().button; }
        uint8_t type() { return data().type; }
    };

    inline bool write_stroke_action(ActionBuffer& b, HeaderMeta hm,
            vec2f position,
            float pressure,
            uint8_t button,
            uint8_t type = 0)
    {
        // get the memory
        size_t data_size = sizeof(StrokeActionData);
        auto w = b.begin_action(data_size);
        if (!w.valid()) return false;

        // write the data
        auto data = w.emplace<StrokeActionData>();
        if (data == nullptr) return false;
        data->position = position;
        data->pressure = pressure;
        data->button = button;
        data->type = type;

        // write the header
        b.end_action<StrokeActionData>(w,hm.timestamp,hm.user);

        return true;
    }

    // Message Action //

    struct MessageActionData {
    public:
        static constexpr ActionType Type = ActionType::User;
    public:
        StringRef message;
        uint16_t  message_id;
    };

    inline bool write_message_action(ActionBuffer& b, HeaderMeta hm,
        const std::string& message,
        uint16_t message_id)
    {
        // get the memory
        size_t string_size = message.length()+1;
        size_t data_size = sizeof(MessageActionData)+string_size;
        auto w = b.begin_action(data_size);
        if (!w.valid()) return false;

        // write the data
        auto data = w.emplace<MessageActionData>();
        if (data == nullptr) return false;
        data->message_id = message_id;
        if (!data->message.write(w, message)) return false;

        // write the header
        b.end_action<MessageActionData>(w,hm.timestamp,hm.user);

        return true;
    }

    struct MessageActionRef : public ActionReference<MessageActionData>
    {
        MessageActionRef() {}
        MessageActionRef(ActionBuffer& b, uint16_t ai) : ActionReference(b,ai) {}

        const sol::StringView message() { return resolve_string(data().message); }
        uint16_t message_id() { return data().message_id; }
    };

}}
