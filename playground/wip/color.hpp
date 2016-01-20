#pragma once

#include "common.hpp"

namespace sol { namespace color {

    struct RGBA {
        float r,g,b,a;
    };

    struct HSVA {
        float h,s,v,a;
    };

    HSVA rgba2hsva(RGBA color);

    RGBA hsva2rgba(HSVA color);

}}
