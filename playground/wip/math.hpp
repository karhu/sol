#pragma once

namespace sol {

    inline float deg2rad(float d) {
        const float PI = 3.14159265358979323846264338327f;
        return d  * PI / 180.0f;
    }

    inline float rad2deg(float r) {
        const float PI = 3.14159265358979323846264338327f;
        return r * 180.0f / PI;
    }

}
