#pragma once

#include "fmt/format.h"

#include <memory>
#include <string>
#include <utility>
namespace db {

/**
 * Table reference types.
 */
enum class TableReferenceType : uint8_t {
	INVALID = 0,         /**< Invalid table reference type. */
	BASE_TABLE = 1,      /**< Base table reference. */
	JOIN = 3,            /**< Output of join. */
	CROSS_PRODUCT = 4,   /**< Output of cartesian product. */
	EXPRESSION_LIST = 5, /**< Values clause. */
	SUBQUERY = 6,        /**< Subquery. */
	CTE = 7,             /**< CTE. */
	EMPTY = 8            /**< Placeholder for empty FROM. */
};

/**
 * A bound table reference.
 */
class BoundTableRef {
public:
	explicit BoundTableRef(TableReferenceType type) : type_(type) {
	}
	BoundTableRef() = default;
	virtual ~BoundTableRef() = default;

	virtual std::string ToString() const {
		switch (type_) {
		case TableReferenceType::INVALID:
			return "";
		case TableReferenceType::EMPTY:
			return "<empty>";
		default:
			std::unreachable();
		}
	}

	auto IsInvalid() const -> bool {
		return type_ == TableReferenceType::INVALID;
	}

	/** The type of table reference. */
	TableReferenceType type_ {TableReferenceType::INVALID};
};

} // namespace db

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<db::BoundTableRef, T>::value, char>>
    : fmt::formatter<std::string> {
	template <typename FormatCtx>
	auto format(const T &x, FormatCtx &ctx) const {
		return fmt::formatter<std::string>::format(x.ToString(), ctx);
	}
};

template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<db::BoundTableRef, T>::value, char>>
    : fmt::formatter<std::string> {
	template <typename FormatCtx>
	auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
		return fmt::formatter<std::string>::format(x->ToString(), ctx);
	}
};

template <>
struct fmt::formatter<db::TableReferenceType> : formatter<string_view> {
	template <typename FormatContext>
	auto format(db::TableReferenceType c, FormatContext &ctx) const {
		string_view name;
		switch (c) {
		case db::TableReferenceType::INVALID:
			name = "Invalid";
			break;
		case db::TableReferenceType::BASE_TABLE:
			name = "BaseTable";
			break;
		case db::TableReferenceType::JOIN:
			name = "Join";
			break;
		case db::TableReferenceType::CROSS_PRODUCT:
			name = "CrossProduct";
			break;
		case db::TableReferenceType::EMPTY:
			name = "Empty";
			break;
		case db::TableReferenceType::EXPRESSION_LIST:
			name = "ExpressionList";
			break;
		case db::TableReferenceType::SUBQUERY:
			name = "Subquery";
			break;
		case db::TableReferenceType::CTE:
			name = "CTE";
			break;
		default:
			name = "Unknown";
			break;
		}
		return formatter<string_view>::format(name, ctx);
	}
};
