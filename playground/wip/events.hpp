#pragma once

namespace sol {

    class Context;

    class Events {
    public:
        void update();
        /* blocking version of update() */
        void wait();
    public:
        bool should_quit() const { return m_should_quit; }
    private:
        Events(Context& context);
        ~Events();
    private:
        bool m_should_quit = false;
        Context& m_context;
    private:
        friend class Context;
    };

}
