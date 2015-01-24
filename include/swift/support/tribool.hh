/**
 * Copyright © 2014 Saleem Abdulrasool <compnerd@compnerd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#ifndef swift_support_tribool_hh
#define swift_support_tribool_hh

class tribool {
  friend bool indeterminate(tribool value) noexcept;
  friend tribool operator!(tribool value) noexcept;

  enum value { false_value, true_value, indeterminate_value } value_;

public:
  struct indeterminate_type {};
  static constexpr const indeterminate_type indeterminate = {};

  tribool() noexcept : value_(value::false_value) {}
  tribool(bool value) noexcept : value_(value ? true_value : false_value) {}
  tribool(indeterminate_type) noexcept : value_(indeterminate_value) {}

  operator bool() const noexcept {
    return value_ == true_value;
  }
};

inline bool indeterminate(tribool value) noexcept {
  return value.value_ == tribool::indeterminate_value;
}

inline tribool operator!(tribool value) noexcept {
  switch (value.value_) {
  default:
    return tribool(tribool::indeterminate);
  case tribool::false_value:
    return tribool(tribool::true_value);
  case tribool::true_value:
    return tribool(tribool::false_value);
  }
}

inline tribool operator&&(tribool lhs, tribool rhs) noexcept {
  if (static_cast<bool>(lhs) and static_cast<bool>(rhs))
    return true;
  if (static_cast<bool>(!lhs) or static_cast<bool>(!rhs))
    return false;
  return tribool::indeterminate;
}

inline tribool operator&&(tribool lhs, bool rhs) noexcept {
  return rhs ? lhs : tribool(false);
}

inline tribool operator&&(bool lhs, tribool rhs) noexcept {
  return lhs ? rhs : tribool(false);
}

inline tribool operator&&(tribool::indeterminate_type, tribool rhs) noexcept {
  return static_cast<bool>(!rhs) ? tribool(false) : tribool::indeterminate;
}

inline tribool operator&&(tribool lhs, tribool::indeterminate_type) noexcept {
  if (static_cast<bool>(!lhs))
    return false;
  return tribool::indeterminate;
}

inline tribool operator||(tribool lhs, tribool rhs) noexcept {
  if (static_cast<bool>(lhs) or static_cast<bool>(rhs))
    return true;
  if (static_cast<bool>(!lhs) and static_cast<bool>(!rhs))
    return false;
  return tribool::indeterminate;
}

inline tribool operator||(tribool lhs, bool rhs) noexcept {
  return rhs ? tribool(true) : lhs;
}

inline tribool operator||(bool lhs, tribool rhs) noexcept {
  return lhs ? tribool(true) : rhs;
}

inline tribool operator||(tribool::indeterminate_type, tribool rhs) noexcept {
  return static_cast<bool>(rhs) ? tribool(true) : tribool::indeterminate;
}

inline tribool operator||(tribool lhs, tribool::indeterminate_type) noexcept {
  return static_cast<bool>(lhs) ? tribool(true) : tribool::indeterminate;
}

inline tribool operator==(tribool lhs, tribool rhs) noexcept {
  if (indeterminate(lhs) or indeterminate(rhs))
    return tribool::indeterminate;
  return static_cast<bool>(lhs) == static_cast<bool>(rhs);
}

inline tribool operator==(tribool lhs, bool rhs) noexcept {
  return indeterminate(lhs) ? tribool::indeterminate
                            : tribool(static_cast<bool>(lhs) == rhs);
}

inline tribool operator==(bool lhs, tribool rhs) noexcept {
  return indeterminate(rhs) ? tribool::indeterminate
                            : tribool(static_cast<bool>(rhs) == lhs);
}

inline tribool operator==(tribool::indeterminate_type, tribool rhs) noexcept {
  return indeterminate(rhs);
}

inline tribool operator==(tribool lhs, tribool::indeterminate_type) noexcept {
  return indeterminate(lhs);
}

inline tribool operator!=(tribool lhs, tribool rhs) noexcept {
  return not lhs == rhs;
}

inline tribool operator!=(tribool lhs, bool rhs) noexcept {
  return not lhs == rhs;
}

inline tribool operator!=(bool lhs, tribool rhs) noexcept {
  return not lhs == rhs;
}

inline tribool operator!=(tribool::indeterminate_type, tribool rhs) noexcept {
  return not indeterminate(rhs);
}

inline tribool operator!=(tribool lhs, tribool::indeterminate_type) noexcept {
  return not indeterminate(lhs);
}

#endif

