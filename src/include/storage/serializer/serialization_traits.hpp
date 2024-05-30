#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <type_traits>
#include <unordered_set>
namespace db {
class Serializer;   // Forward declare
class Deserializer; // Forward declare
typedef uint16_t field_id_t;
const field_id_t MESSAGE_TERMINATOR_FIELD_ID = 0xFFFF;

template <typename T>
concept HasSerialize = requires(T t, db::Serializer &serializer) {
	{ t.Serialize(serializer) } -> std::same_as<void>;
};

template <typename T>
concept HasDeserializeUniquePtr = requires(db::Deserializer &deserializer) {
	{ T::Deserialize(deserializer) } -> std::same_as<std::unique_ptr<T>>;
};

template <typename T>
concept HasDeserializeSharedPtr = requires(db::Deserializer &deserializer) {
	{ T::Deserialize(deserializer) } -> std::same_as<std::shared_ptr<T>>;
};

template <typename T>
concept HasDeserializeValue = requires(db::Deserializer &deserializer) {
	{ T::Deserialize(deserializer) } -> std::same_as<T>;
};

// Combined concept
template <typename T>
concept HasDeserialize = HasDeserializeUniquePtr<T> || HasDeserializeSharedPtr<T> || HasDeserializeValue<T>;

template <typename T>
concept IsUniquePtr = requires {
	typename T::element_type;
	requires std::same_as<T, std::unique_ptr<typename T::element_type>>;
};

template <typename T>
concept IsSharedPtr = requires {
	typename T::element_type;
	requires std::same_as<T, std::shared_ptr<typename T::element_type>>;
};

template <typename T>
concept IsOptionalPtr = requires {
	typename T::value_type;
	requires std::same_as<T, std::optional<typename T::value_type>>;
};

template <typename T>
concept IsPair = requires {
	typename T::first_type;
	typename T::second_type;
	requires std::same_as<T, std::pair<typename T::first_type, typename T::second_type>>;
};

template <typename T>
concept IsUnorderedSet = requires {
	typename T::value_type;
	requires std::same_as<T, std::unordered_set<typename T::value_type>>;
};

template <typename T>
concept IsUnorderedMap = requires {
	typename T::key_type;
	typename T::mapped_type;
	requires std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type>>;
};

template <typename T>
concept IsMap = requires {
	typename T::key_type;
	typename T::mapped_type;
	typename T::key_compare;
	typename T::allocator_type;
	requires std::same_as<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare,
	                                  typename T::allocator_type>>;
};

template <typename T>
concept IsSet = requires {
	typename T::key_type;
	requires std::same_as<T, std::set<typename T::key_type>>;
};

template <typename T>
concept IsAtomic = requires {
	typename T::value_type;
	requires std::same_as<T, std::atomic<typename T::value_type>>;
};

template <typename T>
concept IsVector = requires {
	typename T::value_type;
	requires std::same_as<T, std::vector<typename T::value_type>>;
};
}; // namespace db
