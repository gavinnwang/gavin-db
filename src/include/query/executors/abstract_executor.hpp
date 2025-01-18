#pragma once

#include "query/executor_context.hpp"
#include "storage/table/tuple.hpp"

#include <memory>
namespace db {

class AbstractExecutor {
public:
	// virtual void Init() = 0;
	// Yield the next tuple from this executor.
	explicit AbstractExecutor(const ExecutorContext &exec_ctx) : exec_ctx_ {exec_ctx} {
	}

	virtual bool Next(Tuple &tuple, RID &rid) = 0;

	[[nodiscard]] virtual const Schema &GetOutputSchema() const = 0;

	virtual ~AbstractExecutor() = default;

protected:
	const ExecutorContext &exec_ctx_;
};
} // namespace db
