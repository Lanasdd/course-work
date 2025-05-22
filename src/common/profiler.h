#pragma once
#include <chrono>
#include <string>
#include <iostream>

template <typename TClock> typename TClock::time_point startDuration() {
  return TClock::now();
}

template <typename TClock, typename TDuration>
void stopDuration(std::string_view desc,
                  typename TClock::time_point start_point) {
  using namespace std::chrono;
  std::cerr << desc << ": "
            << duration_cast<TDuration>(TClock::now() - start_point).count()
            << " ms\n";
}

template <typename TDuration>
class DurationLogging {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;
  using Duration = TDuration;

  //explicit DurationLogging(TDesc&& desc_) noexcept(false)
  //    : desc(std::forward<TDesc>(desc_)) {}

  explicit DurationLogging(std::string_view desc_) noexcept(false)
      : desc(desc_) {}

  ~DurationLogging() { stopDuration<Clock, TDuration>(desc, start_point); }
  DurationLogging(const DurationLogging&) = delete;
  DurationLogging& operator=(const DurationLogging&) = delete;
  DurationLogging(DurationLogging&&) = delete;
  DurationLogging& operator=(DurationLogging&&) = delete;

 private:
  TimePoint start_point{Clock::now()};
  std::string_view desc;
};

using DurationLoggingMs = DurationLogging<std::chrono::milliseconds>;

#define DURATION_CONCAT_INTERNAL(X, Y) X ## Y
#define DURATION_CONCAT(X, Y) DURATION_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE DURATION_CONCAT(profileGuard, __LINE__)
#define SCOPE_DURATION_MS(x) DurationLoggingMs UNIQUE_VAR_NAME_PROFILE(x)
