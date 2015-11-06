#pragma once

#include <stdint.h>
#include "vec2.hpp"
#include <vector>
#include <deque>

namespace miro {

    enum class ActionType : uint8_t{
        Unknown = 0,
        StrokeBegin,
        StrokeUpdate,
        StrokeEnd,
    };

    struct StrokeAction {
        vec2f position; // position in the current view frame
        float pressure; // pressure in [0,1]
        uint8_t button; // a button, finger, tip id
    };

    struct Action {
        ActionType type = ActionType::Unknown;
        uint16_t timestamp; // timestamp in 10ms steps, rolls over every 10min
        union Data {
            Data() {}
            StrokeAction stroke;
        }data;
    };

    class IActionSource;
    class IActionSink;

    bool connect(IActionSource& source, IActionSink& sink);
    bool disconnect(IActionSource& source, IActionSink& sink);

    class IActionSource
    {
    public:
        void send(const Action& action);
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
    };


    class IActionSink
    {
    public:
        void receive(Action action);
    protected:
        virtual bool on_connect(IActionSource& source);
        virtual bool on_disconnect(IActionSource& source);
        virtual void on_receive(Action action);
    private:
        bool add_source(IActionSource& sink);
        bool remove_source(IActionSource& sink);
    private:
        std::vector<IActionSource*> m_sources;
    private:
        friend bool connect(IActionSource &source, IActionSink &sink);
        friend bool disconnect(IActionSource &source, IActionSink &sink);
    };

    class BufferingActionSink : public IActionSink {
    public:
        uint32_t count() const;
        Action peak_front() const;
        Action pop_front();
    protected:
        virtual void on_receive(Action action) override;
    private:
        std::deque<Action> m_buffer;
    };
}


