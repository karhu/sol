#pragma once

#include <vec2.hpp>

namespace sol {

    inline float deg2rad(float d) {
        const float PI = 3.14159265358979323846264338327f;
        return d  * PI / 180.0f;
    }

    inline float rad2deg(float r) {
        const float PI = 3.14159265358979323846264338327f;
        return r * 180.0f / PI;
    }

    vec2f barycentric_coordinates(vec2f a, vec2f b, vec2f c, vec2f p) {
        // source: http://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
        vec2f v0 = b - a;
        vec2f v1 = c - a;
        vec2f v2 = p - a;
        float d00 = dot(v0, v0);
        float d01 = dot(v0, v1);
        float d11 = dot(v1, v1);
        float d20 = dot(v2, v0);
        float d21 = dot(v2, v1);
        float denom = d00 * d11 - d01 * d01;
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
        return vec2f(v,w);
    }

    template<typename T>
    T clamp(T value, T min, T max) {
        return value > max ? max :
               value < min ? min :
               value;
    }

}
