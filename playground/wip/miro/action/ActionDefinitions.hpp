#pragma once

#include "vec2.hpp"
#include "Transform2.hpp"

#include "ActionReference.hpp"
#include "ActionBuffer.hpp"

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
        b.end_action<StrokeActionData>(w,hm);

        return true;
    }

    // UserAction //
    struct UserActionData {
    public:
        static constexpr ActionType Type = ActionType::User;
    public:
        enum class Kind : uint8_t {
            Unknown = 0,
            Join = 1,
            Update = 2,
            Leave = 3,
        };

        enum class Flag : uint8_t {
            None = 0,
            Local = 1,
            Spectator = 2,
        };
    public:
        StringRef alias;
        uint16_t  idx;
        Kind      kind;
        Flag      flags;
        // TODO server signature
    };

    inline bool write_user_action(ActionBuffer& b, HeaderMeta hm,
        const std::string& alias, uint16_t idx,
        UserActionData::Kind kind, UserActionData::Flag flags)
    {
        // get the memory
        size_t string_size = alias.length()+1;
        size_t data_size = sizeof(UserActionData)+string_size;
        auto w = b.begin_action(data_size);
        if (!w.valid()) return false;

        // write the data
        auto data = w.emplace<UserActionData>();
        if (data == nullptr) return false;
        data->idx = idx;
        data->kind = kind;
        data->flags = flags;
        if (!data->alias.write(w, alias)) return false;

        // write the header
        b.end_action<UserActionData>(w,hm);

        return true;
    }

    struct UserActionRef : public ActionReference<UserActionData>
    {
        using Kind = UserActionData::Kind;
        using Flag = UserActionData::Flag;

        UserActionRef() {}
        UserActionRef(ActionBuffer& b, uint16_t ai) : ActionReference(b,ai) {}

        const sol::StringView alias() { return resolve_string(data().alias); }
        uint16_t idx() { return data().idx; }
        Kind kind() { return data().kind; }
        Flag flags() { return data().flags; }
    };

    // Viewport Action //

    struct ViewPortActionData {
    public:
        static constexpr ActionType Type = ActionType::Viewport;
    public:
        vec2f   position;
        float   rotation;
        float   scale;
        vec2u16 viewport_dim;
    };

    inline bool write_viewport_action(ActionBuffer& b, HeaderMeta hm,
        const vec2f position,
        const float rotation,
        const float scale,
        const vec2u16 viewport_dim)
    {
        // get the memory
        size_t data_size = sizeof(ViewPortActionData);
        auto w = b.begin_action(data_size);
        if (!w.valid()) return false;

        // write the data
        auto data = w.emplace<ViewPortActionData>();
        if (data == nullptr) return false;
        data->position = position;
        data->rotation = rotation;
        data->scale = scale;
        data->viewport_dim = viewport_dim;

        // write the header
        b.end_action<ViewPortActionData>(w,hm);

        return true;
    }

    struct ViewportActionRef : public ActionReference<ViewPortActionData>
    {
        ViewportActionRef() {}
        ViewportActionRef(ActionBuffer& b, uint16_t ai) : ActionReference(b,ai) {}

        const vec2f&   position() const { return data().position; }
        float rotation() const { return data().rotation; }
        float scale() const { return data().scale; }
        vec2u16 viewport_dim() const { return data().viewport_dim; }
    };

    // Message Action //

    struct MessageActionData {
    public:
        static constexpr ActionType Type = ActionType::Message;
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
        b.end_action<MessageActionData>(w,hm);

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
