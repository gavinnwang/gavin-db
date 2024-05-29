#pragma once

#include <cassert>
#include <iostream>

namespace db {
#define UNREACHABLE(message)  throw std::logic_error(message)
#define ASSERT(expr, message) assert((expr) && (message))
#define ENSURE(expr, message)                                                                                          \
	if (!(expr)) {                                                                                                     \
		std::cerr << "ERROR: " << (message) << std::endl;                                                              \
		std::terminate();                                                                                              \
	}

// Macros to disable copying and moving
#define DISALLOW_COPY(cname)                                                                                           \
	cname(const cname &) = delete;                   /* NOLINT */                                                      \
	auto operator=(const cname &)->cname & = delete; /* NOLINT */

#define DISALLOW_MOVE(cname)                                                                                           \
	cname(cname &&) = delete;                   /* NOLINT */                                                           \
	auto operator=(cname &&)->cname & = delete; /* NOLINT */

#define DISALLOW_COPY_AND_MOVE(cname)                                                                                  \
	DISALLOW_COPY(cname);                                                                                              \
	DISALLOW_MOVE(cname);

} // namespace db
