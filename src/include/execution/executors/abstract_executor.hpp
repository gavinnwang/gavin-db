#pragma once

#include "storage/table/tuple.hpp"
namespace db {

class AbstractExecutor {
public:
	// virtual void Init() = 0;
	// Yield the next tuple from this executor.
	virtual bool Next(Tuple &tuple, RID &rid) = 0;

	virtual const Schema &GetOutputSchema() const = 0;

	virtual ~AbstractExecutor() = default;
};
} // namespace db
