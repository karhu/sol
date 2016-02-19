#pragma once

#include <vector>
#include <deque>

#include "vec2.hpp"
#include "color.hpp"

#include "sol/Slice.hpp"

namespace miro {

    struct StrokePoint
    {
        vec2f position;
        float pressure;
    };

    struct StrokeProperties {
        sol::color::RGBA color;
    };

    struct Stroke
    {
        Stroke();
        Stroke(uint64_t id, uint16_t user_id, bool confirmed, StrokeProperties properties);

        bool finished() const;
        void set_finished(bool value);

        bool confirmed() const;
        void set_confirmed(bool value);

        uint64_t id() const;
        void set_id(uint64_t id);

        void add_point(StrokePoint point);

        uint32_t count() const;
        uint16_t user_id() const;
        const StrokeProperties& properties() const;

        sol::Slice<StrokePoint> points();
     public:
        void build_geometry();
     private:
        uint64_t m_id;
        uint16_t m_user_id;
        bool m_finished = false;
        bool m_confirmed = false;
        StrokeProperties m_properties;
        std::vector<StrokePoint> m_points;
    };

    class StrokeCollection
    {
        using StrokeQueue = std::deque<Stroke>;
    public:
        void begin_stroke(uint16_t user_idx, bool confirmed, StrokeProperties properties);
        void add_point(uint16_t user_idx, StrokePoint point);
        void end_stroke(uint16_t m_user_idx);
        void confirm_stroke(uint16_t user_idx);
    public:
        bool dirty() const;
        void set_dirty(bool value);
    public:
        std::vector<Stroke> remove_old_strokes();
        sol::Slice<Stroke*> strokes();
    private:
        void assert_user_idx(uint16_t user_idx);
        uint64_t generate_next_stroke_id();
    protected:
        uint64_t m_next_stroke_id = 0;
        std::vector<StrokeQueue> m_strokes;
    public:
        std::vector<Stroke*> m_sorted_strokes;
        bool m_dirty = false;
    };

}
