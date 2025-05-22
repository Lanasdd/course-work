#pragma once

#include <string_view>
#include <string>
#include <vector>

namespace string_detail {

using std::literals::string_view_literals::operator""sv;
using std::literals::string_literals::operator""s;

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T trimLeft(T input, typename T::const_pointer trimmed_chars = " "sv.data());

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T trimRight(T input, typename T::const_pointer trimmed_chars = " "sv.data());

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T trim(T input, typename T::const_pointer trimmed_chars = " "sv.data());

template <typename T>
  requires(std::is_base_of_v<std::basic_string_view<typename T::value_type>, T>)
T splitNext(
    T& input,
    typename T::const_pointer separators = ","sv.data(),
    typename T::const_pointer trimmed_chars = " "sv.data());

template <typename Array>
Array split(
    typename Array::value_type input,
    typename Array::value_type::const_pointer separators = " "sv.data());

template <typename Array>
Array splitAndGetSeparator(
    typename Array::value_type input,
    typename Array::value_type::value_type& first_separator,
    typename Array::value_type::const_pointer separators = " "sv.data());

template <typename String, typename Value>
Value toValue(String input);

/*
 * std::string_view trimLeft(
    std::string_view sv, std::string_view trimmed_chars = " "sv);

std::string_view trimRight(
    std::string_view sv, std::string_view trimmed_chars = " "sv);

std::string_view trim(
    std::string_view sv, std::string_view trimmed_chars = " "sv);

std::string_view splitNext(
    std::string_view& sv,
    std::string_view sep_chars = ","sv,
    std::string_view trimmed_chars = " "sv);

template <typename TValue>
TValue to_value(std::string_view sv);
*/
} // namespace string_detail
