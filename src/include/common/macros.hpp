#pragma once

#include <cassert>
#include <iostream>

namespace db {
#define UNREACHABLE(message)  throw std::logic_error(message)
#define ASSERT(expr, message) assert((expr) && (message))
#define ENSURE(expr, message)                                                                                          \
	if (!(expr)) {                                                                                                     \
		std::cerr << "ERROR: " << (message) << std::endl;                                                              \
		exit(1);                                                                                                       \
	} // namespace db

// Macros to disable copying and moving
#define DISALLOW_COPY(cname)                                                                                           \
	cname(const cname &) = delete;                                                                                     \
	cname &operator=(const cname &) = delete;
#define DISALLOW_MOVE(cname)                                                                                           \
	cname(cname &&) = delete;                                                                                          \
	cname &operator=(cname &&) = delete;
#define DISALLOW_COPY_AND_MOVE(cname)                                                                                  \
	DISALLOW_COPY(cname);                                                                                              \
	DISALLOW_MOVE(cname);

} // namespace db
