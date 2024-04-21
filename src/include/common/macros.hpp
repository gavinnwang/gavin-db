#pragma once

#include <cassert>
#define UNREACHABLE(message) throw std::logic_error(message)
#define ASSERT(expr, message) assert((expr) && (message))
