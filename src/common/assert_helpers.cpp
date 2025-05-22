#include "assert_helpers.h"
#include "math_helper.h"

template <typename TStream, typename T>
TStream& output_helper(TStream& stream, const char head, const T& container,
                       const char tail) {
  stream << head;

  constexpr char delimiter[] = ", ";
  bool is_first(true);
  for (const auto& e : container) {
    if (is_first)
      is_first = !is_first;
    else
      stream << delimiter;

    stream << e;
  }

  return stream << tail;
}

template <typename T, typename U>
void assertEqualImpl(const T& t, const U& u, std::string_view t_str,
                     std::string_view u_str, std::string_view file,
                     std::string_view function, unsigned line,
                     std::string_view hint) {
  if (!isEqual(t, u)) {
    std::cout << std::boolalpha << file << "(" << line << "): " << function
              << ": ASSERT_EQUAL(" << t_str << ", " << u_str
              << ") failed: " << t << " != " << u << ".";

    if (!hint.empty()) std::cout << " Hint: " << hint;

    std::cout << '\n';
    abort();
  }
}

template void assertEqualImpl<double, double>(
    const double& t, const double& u, std::string_view t_str,
    std::string_view u_str, std::string_view file, std::string_view function,
    unsigned line, std::string_view hint);

void abortImpl(std::string_view file, const unsigned line,
               std::string_view function, std::string_view expr,
               std::string_view hint) {
  std::cout << file << "(" << line << "): " << function << ": ASSERT(" << expr
            << ") failed.";

  if (!hint.empty()) std::cout << " Hint: " << hint;

  std::cout << '\n';
  abort();
}
