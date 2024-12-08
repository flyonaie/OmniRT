// Copyright (c) 2023, AgiBot Inc.
// All rights reserved.

#pragma once

#include <string>

#include "util/format.h"

namespace aimrt::common::util {

class AimRTException : public std::exception {
 public:
  template <typename... Args>
    // requires std::constructible_from<std::string, Args...>
  AimRTException(Args... args)
      : err_msg_(std::forward<Args>(args)...) {
        static_assert(std::is_constructible<std::string, Args...>::value, "Args must be constructible to std::string");
      }

  ~AimRTException() noexcept override {}

  const char* what() const noexcept override { return err_msg_.c_str(); }

 private:
  std::string err_msg_;
};

}  // namespace aimrt::common::util

#define AIMRT_ASSERT(__expr__, __fmt__, ...)                                                  \
  do {                                                                                        \
    [[unlikely]] if (!(__expr__)) {                                                           \
      throw aimrt::common::util::AimRTException(::aimrt_fmt::format(__fmt__, ##__VA_ARGS__)); \
    }                                                                                         \
  } while (0)
