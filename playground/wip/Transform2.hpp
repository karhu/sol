#pragma once

#include "vec2.hpp"

class Transform2 {
public:
    Transform2() {}
public:
    static Transform2 Identity();
    static Transform2 TSR(vec2f translation, vec2f scale, float rot_d);
    static Transform2 Translation(vec2f t);
    static Transform2 Rotation(float angle_d);
    static Transform2 Scale(vec2f s);
    static Transform2 Scale(float s);
public:
    Transform2 inverse() const;
public:
    Transform2 operator*(const Transform2& next) const;
public:
    const float* data() const;
    float* data();
private:
    float m_data[6];
};


vec2f transform_point(const vec2f& point, const Transform2& t);
