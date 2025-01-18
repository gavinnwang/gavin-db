#pragma once

#include <cassert>
#include <iostream>

namespace db {
#define UNREACHABLE(message) throw std::logic_error(message)

} // namespace db
