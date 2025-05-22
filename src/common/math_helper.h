#pragma once
#include <cmath>

template <typename T>
requires(std::is_floating_point_v<T>) inline bool isEqual(T v1, T v2) {
  return std::fabs(v1 - v2) <=
         (std::fmax<T>(1.0, std::fmax(std::fabs(v1), std::fabs(v2))) *
          std::numeric_limits<T>::epsilon());
}

template <typename T>
requires(std::is_integral_v<T>) inline bool isEqual(T v1, T v2) {
  return v1 == v2;
}
