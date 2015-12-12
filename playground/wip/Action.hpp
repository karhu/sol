#pragma once

#include <stdint.h>
#include "vec2.hpp"
#include <vector>
#include <deque>
#include <mutex>
#include <memory>
#include <tuple>
#include <cstring>
#include <limits>

#include <iostream>

#include "delegate.hpp"

namespace miro {

    namespace actions {

        enum class ActionType : uint8_t{
            Unknown = 0,
            Stroke,
            Viewport,
            User,
        };

        struct DataOffset {
            DataOffset() {}
            DataOffset(uint16_t block, uint16_t byte) : block(block), byte(byte) {}

            uint16_t block = 0;
            uint16_t byte = 0;
        };

        struct RelativeMemoryRange16 {
            uint16_t m_offset = 0;
            uint16_t m_size = 0;

            bool valid() const { return m_size > 0; }
            uint16_t size() const { return m_size; }

            void shorten_front(uint16_t amount) {
                if (amount > m_size){
                    m_offset += m_size;
                    m_size = 0;
                } else {
                    m_offset += amount;
                    m_size -= amount;
                }
            }
        };

        struct HeaderMeta {
            HeaderMeta() {}
            HeaderMeta(uint16_t timestamp, uint8_t user = 0) : timestamp(timestamp), user(user) {}

            uint8_t user = 0;
            uint16_t timestamp = 0;
        };

        struct HeaderData {
            HeaderData() {}
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

        struct MemoryRange {
        public:
            MemoryRange(void* begin = nullptr, size_t size = 0) : m_begin(begin), m_end((char*)begin+size) {}
            ~MemoryRange() = default;

        public:
            void* begin() const { return m_begin; }
            void* end() const { return m_end; }
            size_t size() const { return (char*)m_end - (char*)m_begin; }

            void shorten_front(size_t amount)
            {
                if (size() < amount)
                    m_begin = m_end;
                else
                    m_begin = (char*)m_begin + amount;
            }

        private:
            void* m_begin = 0;
            void* m_end = 0;
        };

        template<typename T, typename S>
        T checked_numeric_cast(S source) {
            using namespace std;
            T target = source;
            if (numeric_limits<T>::max() < numeric_limits<S>::max()) {
                if (S(numeric_limits<T>::max()) < source) {
                    // TODO log error
                    std::cerr << "numeric downcast of too large a value" << std::endl;
                    target = numeric_limits<T>::max();
                }
            }
            if (numeric_limits<T>::min() > numeric_limits<S>::min()) {
                if (S(numeric_limits<T>::min()) > source) {
                    // TODO log error
                    std::cerr << "numeric downcast of too small a value" << std::endl;
                    target = numeric_limits<T>::min();
                }
            }
            return target;
        }

        class ActionBuffer;

        struct RelativeMemoryRange16Writer
        {
            RelativeMemoryRange16Writer() {}
            RelativeMemoryRange16Writer(ActionBuffer& b, RelativeMemoryRange16 memory);

            template<typename T, typename ...Args>
            T* emplace(Args&& ...args) {
                auto size = sizeof(T);
                if (available() < size) return nullptr;
                T* ptr = new(front_ptr()) T(std::forward<Args>(args)...);
                move_front(size);
                return ptr;
            }

            RelativeMemoryRange16 relative_frame() const { return m_memory_range; }
            uint16_t available() const { return m_memory_range.size() - m_front; }
            uint16_t front() const { return m_front; }
            void* front_ptr() const { return m_memory_front; }
            void  move_front(uint16_t v) {
                m_memory_front = ((char*) m_memory_front) + v;
                m_front += v;
            }
            bool valid() const { return m_memory_range.valid(); }
        private:
            RelativeMemoryRange16 m_memory_range;
            uint16_t m_front = 0;
            void* m_memory_front = nullptr;
        };

        struct ActionRef {
            const ActionHeader& header() const;

            template<typename AT>
            AT data() {
                return AT(*m_buffer,m_index);
            }

            MemoryRange data_memory();
        public:
            ActionRef(ActionBuffer& buffer, uint16_t index)
                : m_buffer(&buffer)
                , m_index(index)
            {}

        private:
            ActionBuffer* m_buffer = nullptr;
            uint16_t m_index = 0;
        };

        struct ActionRange
        {
        public:
            uint16_t count() const { return m_end - m_begin; }

            ActionRef get(uint16_t idx) {
                // TODO index assert
                return ActionRef(*m_buffer,idx);
            }
        public:
            ActionBuffer& buffer() { return *m_buffer; }
            uint16_t begin_index() { return m_begin; }
            uint16_t end_index() { return m_end; }
        public:
            ActionRange(ActionBuffer& buffer, uint16_t begin, uint16_t end)
                : m_buffer(&buffer)
                , m_begin(begin)
                , m_end(end)
            {}
        private:
            ActionBuffer* m_buffer = nullptr;
            uint16_t m_begin = 0;
            uint16_t m_end = 0;
        };

        class ActionBuffer {
        public:
            std::vector<ActionHeader> m_headers;
            std::vector<uint8_t> m_data;
            uint16_t m_front = 0;

            uint16_t available() const { return DATA_SIZE - m_front; }
            uint16_t count() const { return m_headers.size(); }
            uint16_t size_headers() const { return m_headers.size() * sizeof(ActionHeader); }
            uint16_t size_data() const { return m_front; }

            uint8_t const* ptr_data() const { return &m_data[0]; }
            ActionHeader const * ptr_headers() const { return &m_headers[0]; }

            void reset() {
                m_front = 0;
                m_data.resize(DATA_SIZE);
                m_headers.clear();
            }

            static constexpr uint16_t DATA_SIZE = 65535u; // uint16::max
        public:
            ActionBuffer()
                : m_data(DATA_SIZE)
            {}
        public:
            RelativeMemoryRange16Writer begin_action(uint16_t size)
            {
                if (available() < size) {
                    return RelativeMemoryRange16Writer();
                }

                RelativeMemoryRange16 range;
                range.m_offset = m_front;
                range.m_size = available();
                return RelativeMemoryRange16Writer(*this,range);
            }

            template<typename ActionT>
            void end_action(RelativeMemoryRange16Writer& writer, int16_t timestamp, int16_t user = 0)
            {
                m_headers.emplace_back();
                auto& h = m_headers.back();
                h.memory = writer.relative_frame();
                h.memory.m_size = writer.front();
                h.meta.user = user;
                h.meta.timestamp = timestamp;
                h.meta.type = ActionT::Type;

                m_front = writer.relative_frame().m_offset + writer.front();
            }

            ActionHeader* allocate_action(size_t data_size)
            {
                // check if we have enough memory available
                if (available() < data_size) {
                    return nullptr;
                }
                ActionHeader* result;

                m_headers.emplace_back();
                result = &m_headers.back();
                result->memory.m_size = (uint16_t)data_size;
                result->memory.m_offset = m_front;
                m_front += data_size;

                return result;
            }

            MemoryRange get_memory(RelativeMemoryRange16 range)
            {
                return MemoryRange(m_data.data()+range.m_offset,range.m_size);
            }

            bool copy_action(ActionBuffer source, uint16_t source_idx)
            {
                auto& h = source.m_headers[source_idx];
                auto mr = source.get_memory(h.memory);
                auto size = mr.size();
                if (available() < size) return false;

                void* dest_begin = &m_data[m_front];
                std::memcpy(dest_begin,mr.begin(),size);
                m_front += mr.size();
                m_headers.push_back(h);
                return true;
            }

            ActionRange copy_action(ActionRange& range) {
                for (uint16_t i = 0; i<range.count(); i++) {
                    uint16_t idx = range.begin_index() + i;
                    if (!copy_action(range.buffer(),idx)) {
                        return ActionRange(range.buffer(), idx, range.end_index());
                    }
                }
                return ActionRange(range.buffer(),0,0);
            }

        public:
            inline ActionType read_action_type(uint16_t action_offset)
            {
                if (action_offset >= m_front)
                    return ActionType::Unknown;
                else
                    return m_headers[action_offset].meta.type;
            }

            inline ActionRange all() {
                return ActionRange(*this,0,m_headers.size());
            }
        };

        inline RelativeMemoryRange16Writer::RelativeMemoryRange16Writer(ActionBuffer& b, RelativeMemoryRange16 memory)
            : m_memory_range(memory)
            , m_memory_front(b.get_memory(memory).begin())
        {}

        struct StringView
        {
            StringView() {}
            StringView(char* front, size_t size)
                : m_front(front), m_end(front+size)
            {}

            char* m_front = nullptr;
            char* m_end = nullptr;

            size_t size() const { return m_end - m_front; }
        };

        struct StringRef {
            int16_t m_local_offset = 0;
            int16_t m_size = 0;
#if 0
            bool write(ActionBuffer& b, RelativeMemoryRange16& mem_ref, uint16_t offset, const std::string& str)
            {
                auto str_size = str.size() + 1;
                RelativeMemoryRange16 mem_ref_str = mem_ref;
                mem_ref_str.shorten_front(offset);
                if (str_size < mem_ref_str.m_size) return false;
                auto mem = b.get_memory(mem_ref_str);
                std::memcpy(mem.begin(),str.c_str(),str_size);
                m_local_offset = offset;
                m_size = str_size;
                return true;
            }
#endif
            bool write(RelativeMemoryRange16Writer& writer, const std::string& str)
            {
                auto str_size = str.size() + 1;
                if (writer.available() < str_size) return false;
                std::memcpy(writer.front_ptr(),str.c_str(),str_size);
                m_local_offset = writer.front();
                m_size = str_size;
                writer.move_front(str_size);
                return true;
            }

            StringView read(ActionBuffer& b, RelativeMemoryRange16 mem_ref)
            {
                mem_ref.shorten_front(m_local_offset);
                if (m_size > mem_ref.m_size) return StringView();
                auto mem = b.get_memory(mem_ref);
                return StringView((char*)mem.begin(), m_size);
            }

        };

        template<typename DataT>
        struct ActionReference
        {
        public:
            ActionReference() {}
            ActionReference(ActionBuffer& buffer, uint16_t action_idx) {
                if (action_idx >= buffer.m_headers.size()) return;
                auto type = buffer.m_headers[action_idx].meta.type;
                if (type != Type) return;

                m_buffer = &buffer;
                m_action_idx = action_idx;
                m_mem_ref = buffer.m_headers[action_idx].memory;
                m_data = (DataT*)buffer.get_memory(m_mem_ref).begin();
            }
        public:
            static constexpr ActionType Type = DataT::Type;
        public:
            bool valid() { return m_buffer != nullptr; }
            const HeaderData& header() { return m_buffer->m_headers[m_action_idx].meta;}
        protected:
            RelativeMemoryRange16 mem_ref() { return m_mem_ref; }
            DataT& data() { return *m_data; }
            StringView resolve_string(StringRef ref) { return ref.read(*m_buffer, m_mem_ref); }
        private:
            ActionBuffer* m_buffer = nullptr;
            DataT* m_data = nullptr;
            uint16_t m_action_idx = 0;
            RelativeMemoryRange16 m_mem_ref;
        };

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

        // TODO: Writer w = start_action(..)

        // bool ok = finish_action<StrokeAction>(w,header_data);

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

            const StringView message() { return resolve_string(data().message); }
            uint16_t message_id() { return data().message_id; }
        };
    }

    namespace v2 {
        enum class ActionType : uint8_t{
            Unknown = 0,
            Stroke,
            Viewport,
            User,
        };

        struct ActionHeader {
            ActionHeader() {}
            ActionHeader(ActionType type) : type(type) {}

            ActionType type = ActionType::Unknown;
            uint16_t timestamp; // timestamp in 10ms steps, rolls over every 10min
            uint16_t user = 0;
        };

        class ActionBase {
        protected:
            ActionBase() : header() {}
            ActionBase(ActionType type) : header(type) {}
        public:
            ActionHeader header;
        };

        template<typename T>
        class ActionImpl : public ActionBase {
        protected:
            ActionImpl() : header(T::Type) {}
        public:
            size_t action_size() const { return sizeof(T); }
            bool   action_verify(size_t buffer_size) const { return true; }
        public:
            ActionHeader header;
        };

        class StrokeAction : public ActionImpl<StrokeAction> {
        public:
            static constexpr ActionType Type = ActionType::Stroke;
        public:
            vec2f position; // position in the current view frame
            float pressure; // pressure in [0,1]
            uint8_t button; // a button, finger, tip id
            uint8_t type = 0; // 0 for update, 1 for begin, 2 for end
        };

        class ViewportAction : public ActionImpl<ViewportAction> {
        public:
            static constexpr ActionType Type = ActionType::Viewport;
        public:
            Transform2f transform;
        };

        class UserAction : public ActionImpl<UserAction> {
        public:
            static constexpr ActionType Type = ActionType::User;
        public:
            size_t action_size() const { return sizeof(UserAction) + name.length(); }
        public:
            enum class Event : uint8_t {
                Unknown = 0,
                Add = 1,
                Remove = 2,
                Change = 3,
            };

            uint16_t user_id;
            Event event;
            bool is_local_user = false;
            std::string name;
        };
    }

    enum class ActionType : uint8_t{
        Unknown = 0,
        StrokeBegin,
        StrokeUpdate,
        StrokeEnd,
        Viewport,
        User,
    };

    struct StrokeAction {
        vec2f position; // position in the current view frame
        float pressure; // pressure in [0,1]
        uint8_t button; // a button, finger, tip id
    };

    struct ViewportAction {
        Transform2f transform;
    };

    struct UserAction {
        enum class Type : uint8_t {
            Unknown = 0,
            Add = 1,
            Remove = 2,
            Change = 3,
        };

        uint16_t user_id;
        Type type;
        bool is_local_user = false;
        //std::string name;
    };

    struct Action {
        ActionType type = ActionType::Unknown;
        uint16_t timestamp; // timestamp in 10ms steps, rolls over every 10min
        uint16_t user = 0;
        union Data {
            Data() {}
            StrokeAction stroke;
            ViewportAction viewport;
            UserAction user;
        }data;
    };

    class IActionSource;
    class IActionSink;

    bool connect(IActionSource& source, IActionSink& sink);
    bool disconnect(IActionSource& source, IActionSink& sink);

    class IActionSource
    {
    protected:
        void send(const Action& action);
        void send(actions::ActionRange action_range);
    protected:
        virtual bool on_connect(IActionSink& sink);
        virtual bool on_disconnect(IActionSink& sink);
    private:
        bool add_sink(IActionSink& sink);
        bool remove_sink(IActionSink& sink);
    private:
        std::vector<IActionSink*> m_sinks;
    private:
        friend bool connect(IActionSource &source, IActionSink &sink);
        friend bool disconnect(IActionSource &source, IActionSink &sink);
        friend class IActionSink;
    };

    class IActionSink
    {
    protected:
        virtual bool on_connect(IActionSource& source);
        virtual bool on_disconnect(IActionSource& source);
        virtual void on_receive(Action action);
        virtual void on_receive(actions::ActionRange action_range);
    private:
        void receive(Action action);
        void receive(actions::ActionRange action_range);
    private:
        bool add_source(IActionSource& sink);
        bool remove_source(IActionSource& sink);
    private:
        std::vector<IActionSource*> m_sources;
    private:
        friend bool connect(IActionSource &source, IActionSink &sink);
        friend bool disconnect(IActionSource &source, IActionSink &sink);
        friend class IActionSource;
    };

    class ActionForwarder : public IActionSink, public IActionSource
    {
    protected:
        virtual void on_receive(Action action) override;
    };

    class ActionSender : public IActionSource {
    public:
        void send(actions::ActionRange action_range) { IActionSource::send(action_range); }
    };

    class BufferingActionSink : public IActionSink {
    public:
        BufferingActionSink();
    public:
        template<typename CB>
        void handle_actions(CB&& cb); // CB: (actions::ActionRange) -> bool

        uint32_t count() const { return m_count; }
    protected:
        virtual void on_receive(actions::ActionRange range) override;
    protected:
        void end_current_write_buffer();
    protected:
        std::unique_ptr<actions::ActionBuffer> m_current_buffer;
        std::deque<std::unique_ptr<actions::ActionBuffer>> m_full_buffers;
        std::vector<std::unique_ptr<actions::ActionBuffer>> m_available_buffers;
        uint32_t m_count = 0;
    };

    template<typename CB>
    void BufferingActionSink::handle_actions(CB &&cb)
    {
        end_current_write_buffer();

        // handle the ranges
        while (!m_full_buffers.empty()) {
            auto buffer = std::move(m_full_buffers.front());
            m_full_buffers.pop_front();
            auto range = buffer->all();
            m_count -= range.count();
            bool cont = cb(range);
            buffer->reset();
            m_available_buffers.push_back(std::move(buffer));
            if (!cont) break;
        }
    }

    class ConcurrentBufferingActionSink : public BufferingActionSink {
    public:
        template<typename CB>
        void handle_actions(CB&& cb); // CB: (actions::ActionRange) -> bool
    public:
        void set_notify_callback(sol::delegate<void()> cb);
    protected:
        virtual void on_receive(actions::ActionRange range) override;
    protected:
        std::mutex m_mutex;
        sol::delegate<void()>  m_notify_cb = nullptr;
    };

    template<typename CB>
    void ConcurrentBufferingActionSink::handle_actions(CB &&cb)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        BufferingActionSink::handle_actions(cb);
    }


    class ConcurrentActionForwarder
            : public ConcurrentBufferingActionSink
            , public IActionSource
    {
    public:
        void poll(); // to be called from the thread that manages connected sinks
    };

    class ActionWriter : public IActionSource {
        actions::ActionBuffer m_buffer;
        uint16_t m_sent_front = 0;
    public:
        actions::ActionBuffer& buffer() { return m_buffer; }
    public:
        uint16_t count_sent() const { return m_sent_front; }
        uint16_t count_written() const { return m_buffer.count(); }
        uint16_t count_ready() const { return count_written() - count_sent(); }

        void reset() {
            m_buffer.reset();
            m_sent_front = 0;
        }

        void send_and_reset() {
            send(actions::ActionRange(m_buffer,count_sent(),count_written()));
            reset();
        }
    };

}


