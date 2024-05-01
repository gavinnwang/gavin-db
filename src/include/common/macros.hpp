#pragma once

#include <cassert>
#include <iostream>

#define UNREACHABLE(message) throw std::logic_error(message)
#define ASSERT(expr, message) assert((expr) && (message))
#define ENSURE(expr, message)                                                  \
  if (!(expr)) {                                                               \
    std::cerr << "ERROR: " << (message) << std::endl;                          \
    std::terminate();                                                          \
  }
