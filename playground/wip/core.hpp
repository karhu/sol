#pragma once

#include <stdint.h>
#include <memory>

namespace arc {

template <typename T>
using uptr = std::unique_ptr<T>;

}
