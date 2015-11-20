#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#define UNUSED(x) (void)(x)

#define NO_COPY(TypeName)                         \
 TypeName(const& TypeName) = delete;              \
 TypeName& operator=(const& TypeName) = delete;   \


