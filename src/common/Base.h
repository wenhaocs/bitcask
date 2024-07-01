#ifndef COMMON_BASE_H_
#define COMMON_BASE_H_

#include <fcntl.h>
#include <fmt/core.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <any>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "common/Logging.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif  // UNUSED

// Log levels
enum LogLevel { FATAL, ERROR, WARNING, INFO, VERBOSE1, VERBOSE2, VERBOSE3, VERBOSE4 };

// Formatted logging
#define FLOG_FATAL(...) LOG(FATAL) << fmt::format(__VA_ARGS__)
#define FLOG_ERROR(...) LOG(ERROR) << fmt::format(__VA_ARGS__)
#define FLOG_WARN(...) LOG(WARNING) << fmt::format(__VA_ARGS__)
#define FLOG_INFO(...) LOG(INFO) << fmt::format(__VA_ARGS__)
#define FVLOG1(...) VLOG(1) << fmt::format(__VA_ARGS__)
#define FVLOG2(...) VLOG(2) << fmt::format(__VA_ARGS__)
#define FVLOG3(...) VLOG(3) << fmt::format(__VA_ARGS__)
#define FVLOG4(...) VLOG(4) << fmt::format(__VA_ARGS__)

// namespace ProjectExample {

using VariantType = std::variant<int64_t, double, bool, std::string>;

#ifndef VAR_INT64
#define VAR_INT64 0
#endif

#ifndef VAR_DOUBLE
#define VAR_DOUBLE 1
#endif

#ifndef VAR_BOOL
#define VAR_BOOL 2
#endif

#ifndef VAR_STR
#define VAR_STR 3
#endif

// Useful type traits

// Tell if `T' is copy-constructible
template <typename T>
static constexpr auto is_copy_constructible_v = std::is_copy_constructible<T>::value;

// Tell if `T' is move-constructible
template <typename T>
static constexpr auto is_move_constructible_v = std::is_move_constructible<T>::value;

// Tell if `T' is copy or move constructible
template <typename T>
static constexpr auto is_copy_or_move_constructible_v =
    is_copy_constructible_v<T> || is_move_constructible_v<T>;

// Tell if `T' is constructible from `Args'
template <typename T, typename... Args>
static constexpr auto is_constructible_v = std::is_constructible<T, Args...>::value;

// Tell if `U' could be convertible to `T'
template <typename U, typename T>
static constexpr auto is_convertible_v = std::is_constructible<U, T>::value;

// }  // namespace ProjectExample

#endif  // COMMON_BASE_H_
