#include "color.hpp"

#include <algorithm>

namespace sol { namespace color {

HSVA rgba2hsva(RGBA in)
{
    HSVA out;

    float cmax = std::max(std::max(in.r,in.g),in.b);
    float cmin = std::min(std::min(in.r,in.g),in.b);
    float delta = cmax - cmin;

    float hue = 0.0f;
    float sat = 0.0f;

    if (cmax > 0.0f) {
        sat = delta / cmax;
    }

    if (delta == 0) {
        hue = 0.0f;
    } else if ( in.r >= cmax ) {
        hue = (in.g-in.b) / delta;         // between yellow & magenta
    } else if ( in.g >= cmax ) {
        hue = 2.0f + (in.b-in.r) / delta;  // between cyan & yellow
    } else {
        hue = 4.0f + (in.r-in.g) / delta;  // between magenta & cyan
    }

    out.a = in.a;
    out.v = cmax;
    out.h = (hue / 6.0f);
    out.s = sat;
    return out;
}

RGBA hsva2rgba(HSVA in)
{
    RGBA out;
    out.a = in.a;

    if(in.s <= 0.0f)
    {
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }

    float hue = in.h * 6;
    int hue_i = (int) hue;
    float ff = hue - hue_i;
    float p = in.v * (1.0 - in.s);
    float q = in.v * (1.0 - (in.s * ff));
    float t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(hue_i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

}}
