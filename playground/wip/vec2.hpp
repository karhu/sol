#pragma once

#include <cmath>
#include <stdint.h>

template <typename T>
struct vec2 {
    T x,y;
public:
    vec2() {}
    vec2(T x, T y) : x(x), y(y) {}
    ~vec2() = default;

    template<typename S>
    explicit vec2(const vec2<S>& other) : x(other.x) , y(other.y) {}
public:
    inline vec2 operator-() const {
        return vec2{-x,-y};
    }
    inline vec2 operator-(const vec2& rhs) const {
        return vec2(x-rhs.x,y-rhs.y);
    }
    inline vec2 operator+(const vec2& rhs) const {
        return vec2(x+rhs.x,y+rhs.y);
    }
    template<typename S>
    inline vec2 operator/(const vec2<S>& rhs) const {
        return vec2(x/rhs.x,y/rhs.y);
    }
    inline vec2 operator/(T rhs) const {
        return vec2(x/rhs,y/rhs);
    }
    template<typename S>
    inline vec2<T> operator*(const vec2<S>& rhs) const {
        return vec2<T>(x*rhs.x,y*rhs.y);
    }
    inline vec2 operator*(T rhs) const {
        return vec2(x*rhs,y*rhs);
    }
};

template<typename T>
inline vec2<T> operator*(const T& lhs, const vec2<T>& rhs) {
    return rhs * lhs;
}

using vec2f = vec2<float>;
using vec2u32 = vec2<uint32_t>;
using vec2u16 = vec2<uint16_t>;

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

template<typename T, typename Q>
inline auto dot(const vec2<T>& a, const vec2<Q>& b) {
    return a.x*b.x + a.y*b.y;
}

template<typename T>
inline auto length(const vec2<T>& v) {
    return sqrtf(dot(v,v));
}

inline auto normalized(const vec2f v) {
    return v / length(v);
}


