#pragma once

#include "fmt/format.h"

#include <memory>
#include <string>

namespace db {

/** All types of expressions in binder. */
enum class ExpressionType : uint8_t {
	INVALID = 0,    /**< Invalid expression type. */
	CONSTANT = 1,   /**< Constant expression type. */
	COLUMN_REF = 3, /**< A column in a table. */
	TYPE_CAST = 4,  /**< Type cast expression type. */
	FUNCTION = 5,   /**< Function expression type. */
	AGG_CALL = 6,   /**< Aggregation function expression type. */
	STAR = 7,       /**< Star expression type, will be rewritten by binder and won't appear in plan. */
	UNARY_OP = 8,   /**< Unary expression type. */
	BINARY_OP = 9,  /**< Binary expression type. */
	ALIAS = 10,     /**< Alias expression type. */
	FUNC_CALL = 11, /**< Function call expression type. */
	WINDOW = 12,    /**< Window Aggregation expression type. */
};

/** A bound expression. */
class BoundExpression {
public:
	explicit BoundExpression(ExpressionType type) : type_(type) {
	}
	BoundExpression() = default;
	virtual ~BoundExpression() = default;

	[[nodiscard]] virtual std::string ToString() const = 0;

	/** The type of this expression. */
	ExpressionType type_ {ExpressionType::INVALID};
};

} // namespace db

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<db::BoundExpression, T>::value, char>>
    : fmt::formatter<std::string> {
	template <typename FormatCtx>
	auto format(const T &x, FormatCtx &ctx) const {
		return fmt::formatter<std::string>::format(x.ToString(), ctx);
	}
};

template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<db::BoundExpression, T>::value, char>>
    : fmt::formatter<std::string> {
	template <typename FormatCtx>
	auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
		return fmt::formatter<std::string>::format(x->ToString(), ctx);
	}
};

template <>
struct fmt::formatter<db::ExpressionType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(db::ExpressionType c, FormatContext &ctx) const {
		string_view name;
		switch (c) {
		case db::ExpressionType::INVALID:
			name = "Invalid";
			break;
		case db::ExpressionType::CONSTANT:
			name = "Constant";
			break;
		case db::ExpressionType::COLUMN_REF:
			name = "ColumnRef";
			break;
		case db::ExpressionType::TYPE_CAST:
			name = "TypeCast";
			break;
		case db::ExpressionType::FUNCTION:
			name = "Function";
			break;
		case db::ExpressionType::AGG_CALL:
			name = "AggregationCall";
			break;
		case db::ExpressionType::STAR:
			name = "Star";
			break;
		case db::ExpressionType::UNARY_OP:
			name = "UnaryOperation";
			break;
		case db::ExpressionType::BINARY_OP:
			name = "BinaryOperation";
			break;
		case db::ExpressionType::ALIAS:
			name = "Alias";
			break;
		case db::ExpressionType::FUNC_CALL:
			name = "FuncCall";
			break;
		case db::ExpressionType::WINDOW:
			name = "Window";
			break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};
