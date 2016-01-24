#include "Stroke.hpp"

namespace miro {

void StrokeCollection::begin_stroke(uint16_t user_idx, bool confirmed, StrokeProperties properties)
{
    assert_user_idx(user_idx);
    m_strokes[user_idx].emplace_back(generate_next_stroke_id(),confirmed,properties);
}

void StrokeCollection::add_point(uint16_t user_idx, StrokePoint point)
{
    // TODO: replace this by an assert?
    assert_user_idx(user_idx);
    auto& sc = m_strokes[user_idx];
    if (sc.size() > 0 && !sc.back().finished()) {
        sc.back().add_point(point);
    }
}

void StrokeCollection::assert_user_idx(uint16_t user_idx)
{
    if (m_strokes.size() <= user_idx+1)
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
            break;
        }
    }
}

Stroke::Stroke(){}

Stroke::Stroke(uint64_t id, bool confirmed, StrokeProperties properties)
    : m_id(id)
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

std::vector<StrokePoint> &Stroke::points()
{
    return m_points;
}

}
