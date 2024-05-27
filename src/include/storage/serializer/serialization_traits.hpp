#pragma once

#include <cstdint>
#include <type_traits>
namespace db {
class Serializer;   // Forward declare
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
};
