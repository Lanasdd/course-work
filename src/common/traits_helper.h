#pragma once

template<typename ...>
using void_t=void;

template<typename T, class=void> struct is_container : std::false_type {};

template<typename T>
struct is_container<T, void_t<typename T::iterator>> : std::true_type {};

