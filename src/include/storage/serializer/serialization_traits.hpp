#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <type_traits>
#include <unordered_set>
namespace db {
class Serializer; // Forward declare
typedef uint16_t field_id_t;

template <typename T, typename = T>
struct has_serialize : std::false_type {};
template <typename T>
struct has_serialize<
    T, typename std::enable_if<
           std::is_same<decltype(std::declval<T>().Serialize(std::declval<db::Serializer &>())), void>::value, T>::type>
    : std::true_type {};

template <typename T, typename = T>
struct has_deserialize : std::false_type {};

template <typename T>
struct is_unique_ptr : std::false_type {};
template <typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {
	typedef T ELEMENT_TYPE;
};

template <typename T>
struct is_shared_ptr : std::false_type {};
template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {
	typedef T ELEMENT_TYPE;
};

template <typename T>
struct is_optional_ptr : std::false_type {};
template <typename T>
struct is_optional_ptr<std::optional<T>> : std::true_type {
	typedef T ELEMENT_TYPE;
};

template <typename T>
struct is_pair : std::false_type {};
template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type {
	typedef T FIRST_TYPE;
	typedef U SECOND_TYPE;
};

template <typename T>
struct is_unordered_set : std::false_type {};
template <typename... Args>
struct is_unordered_set<std::unordered_set<Args...>> : std::true_type {
	typedef typename std::tuple_element<0, std::tuple<Args...>>::type ELEMENT_TYPE;
	typedef typename std::tuple_element<1, std::tuple<Args...>>::type HASH_TYPE;
	typedef typename std::tuple_element<2, std::tuple<Args...>>::type EQUAL_TYPE;
};

// Check if T is a unordered map, and provide access to the inner type
template <typename T>
struct is_unordered_map : std::false_type {};
template <typename... Args>
struct is_unordered_map<typename std::unordered_map<Args...>> : std::true_type {
	typedef typename std::tuple_element<0, std::tuple<Args...>>::type KEY_TYPE;
	typedef typename std::tuple_element<1, std::tuple<Args...>>::type VALUE_TYPE;
	typedef typename std::tuple_element<2, std::tuple<Args...>>::type HASH_TYPE;
	typedef typename std::tuple_element<3, std::tuple<Args...>>::type EQUAL_TYPE;
};

template <typename T>
struct is_map : std::false_type {};
template <typename... Args>
struct is_map<typename std::map<Args...>> : std::true_type {
	typedef typename std::tuple_element<0, std::tuple<Args...>>::type KEY_TYPE;
	typedef typename std::tuple_element<1, std::tuple<Args...>>::type VALUE_TYPE;
	typedef typename std::tuple_element<2, std::tuple<Args...>>::type HASH_TYPE;
	typedef typename std::tuple_element<3, std::tuple<Args...>>::type EQUAL_TYPE;
};

template <typename T>
struct is_set : std::false_type {};
template <typename... Args>
struct is_set<std::set<Args...>> : std::true_type {
	typedef typename std::tuple_element<0, std::tuple<Args...>>::type ELEMENT_TYPE;
	typedef typename std::tuple_element<1, std::tuple<Args...>>::type HASH_TYPE;
	typedef typename std::tuple_element<2, std::tuple<Args...>>::type EQUAL_TYPE;
};

template <typename T>
struct is_atomic : std::false_type {};

template <typename T>
struct is_atomic<std::atomic<T>> : std::true_type {
	typedef T TYPE;
};
// Check if T is a vector, and provide access to the inner type
template <typename T>
struct is_vector : std::false_type {};
template <typename T>
struct is_vector<typename std::vector<T>> : std::true_type {
	typedef T ELEMENT_TYPE;
};
}; // namespace db
