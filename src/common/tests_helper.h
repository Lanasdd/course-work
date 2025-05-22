#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <tuple>

#include "profiler.h"

namespace tests {
template <typename TFunc>
void RunTestImpl(const TFunc func, const char func_name[]) {
  func();
  std::cerr << func_name << ": (result = OK)\n";  
}

template <typename TFunc>
void RunTestWithResultAndDurationLoggingImpl(const char func_name[], TFunc func) {
  using Clock = DurationLoggingMs::Clock;
  auto timer1{Clock::now()};
  auto res = func();
  auto timer2{Clock::now()};

  std::cerr << func_name << ": "
      << duration_cast<DurationLoggingMs::Duration>(timer2 - timer1).count()
      << " ms (return = " << res << "; result = OK)\n";
}

template <typename TFunc>
void RunTestWithParamsAndDurationLoggingImplEx(const char func_name[], TFunc func, const auto&& par) {
  std::cerr << "\n-------\nStart " << func_name << "\n";
  using Clock = DurationLoggingMs::Clock;
  auto timer1{Clock::now()};
  std::apply(
      [&](auto&&... args) { func(std::forward<decltype(args)>(args)...); },
      par);  
  auto timer2{Clock::now()};

  std::cerr << func_name << ": "
            << duration_cast<DurationLoggingMs::Duration>(timer2 - timer1).count()
            << " ms (result = OK)\n";
}

#define STRINGIFY(x) #x
#define CONCAT_STR(X, Y) STRINGIFY(X##Y)

#define RUN_TEST(func) tests::RunTestImpl((func), #func)

#define RUN_TEST_WITH_DURATION_LOG_EX(func)                    \
  tests::RunTestWithParamsAndDurationLoggingImplEx(#func, func, \
                                                   std::make_tuple())

#define RUN_TEST_WITH_RESULT_AND_DURATION_LOG(func) \
  tests::RunTestWithResultAndDurationLoggingImpl(#func, func)

#define RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX(func, par) \
  tests::RunTestWithParamsAndDurationLoggingImplEx(#func, func, par)

#define RUN_TEST_WITH_PARAMS_AND_DURATION_LOG_EX_SUF(func, suf, par) \
  tests::RunTestWithParamsAndDurationLoggingImplEx(CONCAT_STR(func, suf), func, par)

}  // namespace tests
