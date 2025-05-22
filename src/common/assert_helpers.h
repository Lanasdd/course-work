#pragma once
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cassert>


[[maybe_unused]]
 constexpr auto Check(auto e) {
#ifndef NDEBUD
  assert(e);
#endif
  return e;
}

 [[maybe_unused]]
constexpr bool CheckPositive(bool expr) {
#ifndef NDEBUD
  assert(expr);
#endif
  return expr;
}

[[maybe_unused]]
constexpr bool CheckNegative(bool expr) {
#ifndef NDEBUD
  assert(!expr);
#endif
  return expr;
}

template <typename TStream, typename TKey, typename TVal>
TStream& operator<<(TStream& stream, const std::pair<TKey, TVal>& val) {
  return stream << val.first << ": " << val.second;
}

template <typename TStream, typename T>
TStream& output_helper(TStream& stream, const char head, const T& container,
                       const char tail);

template <typename TStream, typename T>
TStream& operator<<(TStream& stream, const std::vector<T>& container) {
  return output_helper(stream, '[', container, ']');
}

template <typename TStream, typename T>
TStream& operator<<(TStream& stream, const std::set<T>& container) {
  return output_helper(stream, '{', container, '}');
}

template <typename TStream, typename TKey, typename TVal>
TStream& operator<<(TStream& stream, const std::map<TKey, TVal>& container) {
  return output_helper(stream, '{', container, '}');
}

template <typename T, typename U>
void assertEqualImpl(const T& t, const U& u, std::string_view t_str,
                     std::string_view u_str, std::string_view file,
                     std::string_view function, unsigned line,
                     std::string_view hint);

#define ASSERT_EQUAL_HINT(a, b, hint) \
  assertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_EQUAL(a, b) ASSERT_EQUAL_HINT(a, b, std::string_view{})

[[noreturn]]
void abortImpl(std::string_view file, unsigned line,
               std::string_view function, std::string_view expr,
               std::string_view hint = std::string_view{});

#define ASSERT_HINT(expr, hint) \
  if (!(expr)) abortImpl(__FILE__, __LINE__, __FUNCTION__, #expr, (hint))

#define ASSERT(expr) ASSERT_HINT(expr, std::string_view{})
