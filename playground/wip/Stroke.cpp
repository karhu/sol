#include "Stroke.hpp"

#include <algorithm>

#include "math.hpp"

namespace miro {

void StrokeCollection::begin_stroke(uint16_t user_idx, bool confirmed, StrokeProperties properties)
{
    assert_user_idx(user_idx);
    m_strokes[user_idx].emplace_back(generate_next_stroke_id(),user_idx, confirmed, properties);
    m_sorted_strokes.push_back(&m_strokes[user_idx].back());
}

void StrokeCollection::add_point(uint16_t user_idx, StrokePoint point)
{
    // TODO: replace this by an assert?
    assert_user_idx(user_idx);
    auto& sc = m_strokes[user_idx];
    if (sc.size() > 0 && !sc.back().finished()) {
        sc.back().add_point(point);
        m_dirty = true;
    }
}

void StrokeCollection::assert_user_idx(uint16_t user_idx)
{
    if (m_strokes.size() <= user_idx)
        m_strokes.resize(user_idx+1);
}

uint64_t StrokeCollection::generate_next_stroke_id()
{
    return m_next_stroke_id++;
}

void StrokeCollection::end_stroke(uint16_t user_idx)
{
    assert_user_idx(user_idx);
    m_strokes[user_idx].back().set_finished(true);
}

void StrokeCollection::confirm_stroke(uint16_t user_idx)
{
    assert_user_idx(user_idx);
    for (auto& stroke : m_strokes[user_idx]) {
        if (!stroke.confirmed()) {
            stroke.set_id(generate_next_stroke_id());
            stroke.set_confirmed(true);
            // remove from list and add to back
            m_sorted_strokes.erase(std::remove(
                m_sorted_strokes.begin(),
                m_sorted_strokes.end(), &stroke),
                m_sorted_strokes.end());
            m_sorted_strokes.push_back(&stroke);
            break;
        }
    }
}

bool StrokeCollection::dirty() const
{
    return m_dirty;
}

void StrokeCollection::set_dirty(bool value)
{
    m_dirty = value;
}

std::vector<Stroke> StrokeCollection::remove_old_strokes()
{
    std::vector<Stroke> old;

    if (m_sorted_strokes.empty()) return old;

    // keep to newest strokes up to a certain number of
    // stroke points
    const uint32_t MAX_POINT_COUNT = 250;
    uint32_t point_count = 0;
    int32_t last_i = -1;
    for (int i=m_sorted_strokes.size()-1; i>=0; i--) {
        auto s = m_sorted_strokes[i];
        point_count += s->count();
        if (point_count > MAX_POINT_COUNT) {
            last_i = i;
            break;
        }
    }

    // if there's still unconfirmed ones
    // we'll wait
    bool confirmed = true;
    for (int i=0; i<=last_i; i++) {
        auto s = m_sorted_strokes[i];
        confirmed = confirmed && s->confirmed();
    }
    if (!confirmed) return old;

    // if there's unfinished ones, we'll interrupt them
    for (int i=0; i<=last_i; i++) {
        auto s = m_sorted_strokes[i];
        if (!s->finished()) {
            s->set_finished(true);
            begin_stroke(s->user_id(),true,s->properties());
        }
    }

    // move them away
    for (int i=0; i<=last_i; i++) {
        auto s = m_sorted_strokes[i];
        old.emplace_back(std::move(*s));
        s = &old.back();
        m_strokes[s->user_id()].pop_front();
    }
    // remove the first last_i+1 elements from m_sorted_strokes
    std::vector<Stroke*>(m_sorted_strokes.begin()+last_i+1, m_sorted_strokes.end()).swap(m_sorted_strokes);

    return old;
}

sol::Slice<Stroke *> StrokeCollection::strokes()
{
    return sol::make_slice(&m_sorted_strokes.front(),m_sorted_strokes.size());
}

Stroke::Stroke(){}

Stroke::Stroke(uint64_t id, uint16_t user_id, bool confirmed, StrokeProperties properties)
    : m_id(id)
    , m_user_id(user_id)
    , m_confirmed(confirmed)
    , m_properties(properties)
{}

bool Stroke::finished() const
{
    return m_finished;
}

bool Stroke::confirmed() const
{
    return m_confirmed;
}

void Stroke::set_finished(bool value)
{
    m_finished = value;
}

void Stroke::set_confirmed(bool value)
{
    m_confirmed = value;
}

uint64_t Stroke::id() const
{
    return m_id;
}

void Stroke::set_id(uint64_t id)
{
    m_id = id;
}

void Stroke::add_point(StrokePoint point)
{
    m_points.push_back(point);
}

uint32_t Stroke::count() const
{
    return m_points.size();
}

uint16_t Stroke::user_id() const
{
    return m_user_id;
}

const StrokeProperties &Stroke::properties() const
{
    return m_properties;
}

sol::Slice<StrokePoint> Stroke::points()
{
    return sol::make_slice(m_points.data(),m_points.size());
}

void Stroke::build_geometry()
{
#if 0
    //TODO
    if (m_points.size() == 0) return;
    if (m_points.size() == 1) {
        // TODO create circle
        return;
    }

    auto radius = [=](StrokePoint& pt) {
        return 10.0f*pt.pressure;
    };

    struct CrossVerts {
        vec2f left;
        vec2f center;
        vec2f right;
    };

    struct LineBlock {
        CrossVerts begin;
        CrossVerts end;

        vec2f dir() { return end.center - begin.center; }
    };

    auto lb = [=](int i) {
        auto first = m_points[i];
        auto second = m_points[i+1];
        auto delta = second.position - first.position;
        delta = normalized(delta);

        LineBlock block{
            CrossVerts{
                first.position + radius(first)*vec2f{-delta.y,  delta.x},
                first.position,
                first.position + radius(first)*vec2f{ delta.y, -delta.x}
            },
            CrossVerts{
                second.position + radius(second)*vec2f{-delta.y,  delta.x},
                second.position,
                second.position + radius(second)* vec2f{ delta.y, -delta.x}
            }
        };
    };

    std::vector<LineBlock> blocks;
    blocks.push_back(lb(0));

    using namespace sol::math;

    for (int i=1; i<m_points.size()-1; i++) {
        auto& last_line_block = blocks.back();
        auto  next_line_block = lb(i);
        // TODO intersect
        float v = cross(last_line_block.dir(),next_line_block.dir());
        if (v > 0.0f) // right turn ?
        {
            float t_last,t_next;
            auto d_last = last_line_block.first.right - last_line_block.second.right;
            auto d_next = next_line_block.second.right - next_line_block.first.right;
            bool intersect = intersect_lines(
                last_line_block.second.right, d_last,
                next_line_block.first.right, d_next,
                t_last,t_next);
            if (intersect) {
                last_line_block.second.right += t_last * d_last;
                next_line_block.first.right += t_next * d_next;
            } else {

            }

        } else if (v < 0.0f) { // left turn?

        } else {
            // TODO
        }
    }

    // TODO add arc segments
#endif
}

}
