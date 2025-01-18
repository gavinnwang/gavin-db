#pragma once

#include <cassert>
#include <iostream>

namespace db {
#define UNREACHABLE(message)  throw std::logic_error(message)
#define ASSERT(expr, message) assert((expr) && (message))

} // namespace db
