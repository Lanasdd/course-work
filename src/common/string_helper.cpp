#include "string_helper.h"
#include <charconv>

namespace string_detail {

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T trimLeft(T input, typename T::const_pointer trimmed_chars) {
  input.remove_prefix(
      std::min(input.find_first_not_of(trimmed_chars), input.size()));
  return input;
}

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T trimRight(T input, typename T::const_pointer trimmed_chars) {
  if (auto pos{input.find_last_not_of(trimmed_chars)}; pos != input.npos) {
    input.remove_suffix(input.size() - (pos + 1));
  }
  return input;
}

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T trim(T input, typename T::const_pointer trimmed_chars) {
  return trimRight(trimLeft(input, trimmed_chars), trimmed_chars);
}

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T splitNext(
    T& input,
    typename T::const_pointer separators,
    typename T::const_pointer trimmed_chars) {
  T ret;

  trimLeft(input);
  if (auto pos{input.find(separators)}; pos != input.npos) {
    ret = trimRight(input.substr(0, pos), trimmed_chars);
    input.remove_prefix(pos + 1);
  }

  return ret;
}

template <typename Array>
Array split(
    typename Array::value_type input,
    typename Array::value_type::const_pointer separators) {
  Array parts;

  size_t part_length{0};
  while ((part_length = input.find(separators)) != input.npos) {
    parts.emplace_back(input.data(), part_length);
    input.remove_prefix(part_length + 1);
  }

  if (!input.empty()) {
    parts.push_back(std::move(input));
  }

  return parts;
}

template <typename Array>
Array splitAndGetSeparator(
    typename Array::value_type input,
    typename Array::value_type::value_type& first_separator,
    typename Array::value_type::const_pointer separators) {
  Array parts;

  size_t part_length{0};
  first_separator = static_cast<typename Array::value_type::value_type>(0);

  while ((part_length = input.find(separators)) != input.npos) {
    if (!first_separator)
      first_separator = input[part_length];

    parts.emplace_back(input.data(), part_length);
    input.remove_prefix(part_length + 1);
  }

  if (!input.empty()) {
    parts.push_back(std::move(input));
  }

  return parts;
}

template <typename String, typename Value>
Value toValue(String input) {
  Value ret;
  std::from_chars(input.data(), input.data() + input.size(), ret);
  return ret;
}

/*
std::string_view trimLeft(
    std::string_view sv, std::string_view trimmed_chars = " "sv) {
  sv.remove_prefix(std::min(sv.find_first_not_of(trimmed_chars), sv.size()));
  return sv;
}

std::string_view trimRight(
    std::string_view sv, std::string_view trimmed_chars = " "sv) {
  if (auto pos{sv.find_last_not_of(trimmed_chars)}; pos != sv.npos) {
    sv.remove_suffix(sv.size() - (pos + 1));
  }

  return sv;
}

std::string_view trim(
    std::string_view sv, std::string_view trimmed_chars = " "sv) {
  return trimRight(trimLeft(sv, trimmed_chars), trimmed_chars);
}

std::string_view splitNext(
    std::string_view& sv,
    std::string_view separators = ","sv,
    std::string_view trimmed_chars = " "sv) {
  std::string_view ret_sv;

  trimLeft(sv);
  if (auto pos{sv.find(separators)}; pos != sv.npos) {
    ret_sv = trimRight(sv.substr(0, pos), trimmed_chars);
    sv.remove_prefix(pos + 1);
  }

  return ret_sv;
}

inline std::vector<std::string_view> split(
    std::string_view input, const char separator) {
  std::vector<std::string_view> parts;

  size_t part_length = 0;
  while ((part_length = input.find(separator)) != input.npos) {
    parts.emplace_back(input.data(), part_length);
    input.remove_prefix(part_length + 1);
  }

  if (!input.empty()) {
    parts.push_back(std::move(input));
  }

  return parts;
}

template <typename TValue>
TValue toValue(std::string_view sv) {
  TValue ret;
  std::from_chars(sv.data(), sv.data() + sv.size(), ret);
  return ret;
}
*/

////////////////////////////////////////////////////////////////////////////////
/// template instantiate

template std::string_view trimLeft<std::string_view>(
    std::string_view input, char const* trimmed_chars);

template std::string_view trimRight<std::string_view>(
    std::string_view input, char const* trimmed_chars);

template std::string_view trim<std::string_view>(
    std::string_view input, typename std::string_view::const_pointer trimmed_chars);

template std::string_view splitNext<std::string_view>(
    std::string_view& input, char const* separators, char const* trimmed_chars);

template std::vector<std::string_view> split(
    std::string_view input, char const* separators);

template std::vector<std::string_view> splitAndGetSeparator(
    std::string_view input,
    char& first_separator,
    char const* separators);

template double toValue<std::string_view, double>(std::string_view input);

} // namespace string_detail

//#include "string_helper.impl"
