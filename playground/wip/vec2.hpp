#pragma once

#include <cmath>

struct vec2f {
    vec2f() {}

    vec2f(float x, float y)
        : x(x)
        , y(y)
    {}

    vec2f operator-() const {
        return vec2f(-x,-y);
    }

    vec2f operator-(const vec2f& rhs) const {
        return vec2f(x-rhs.x,y-rhs.y);
    }

    vec2f operator+(const vec2f& rhs) const {
        return vec2f(x+rhs.x,y+rhs.y);
    }

    vec2f operator/(const vec2f& rhs) const {
        return vec2f(x/rhs.x,y/rhs.y);
    }

    vec2f operator/(float rhs) const {
        return vec2f(x/rhs,y/rhs);
    }

    vec2f operator*(const vec2f& rhs) const {
        return vec2f(x*rhs.x,y*rhs.y);
    }

    vec2f operator*(float rhs) const {
        return vec2f(x*rhs,y*rhs);
    }

    float x,y;
};

struct BoundingBox2f {
    vec2f min, max;

    inline BoundingBox2f(vec2f min, vec2f max)
        : min(min), max(max) {}

    inline BoundingBox2f() {}

    inline vec2f dimension() const {
        return max - min;
    }

    inline vec2f transform_local(vec2f global) const {
        return (global - min) / dimension();
    }
};

struct Transform2f {
    vec2f translation = {0,0};
    vec2f scale = {1,1};
    float rotation = 0; // radians, clockwise
};

inline vec2f transform_point(const Transform2f& t, const vec2f& p) {
    float ca = cos(t.rotation);
    float sa = sin(t.rotation);

    vec2f rot{
         ca*p.x - sa*p.y,
         sa*p.x + ca*p.y
    };

    return (rot * t.scale) + t.translation;
}

inline Transform2f inverse(const Transform2f& t) {
    Transform2f it;
    it.scale = vec2f{1.0,1.0}/t.scale;
    it.rotation = - t.rotation;

    float ca = cos(it.rotation);
    float sa = sin(it.rotation);

    vec2f itranslation{
        ca*t.translation.x - sa*t.translation.y,
        sa*t.translation.x + ca*t.translation.y
    };

    it.translation = itranslation * -it.scale;
    return it;
}




