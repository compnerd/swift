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

#ifndef swift_parser_result_hh
#define swift_parser_result_hh

#include "swift/support/type_traits.hh"

namespace swift {
namespace parse {
template <typename Type>
class result {
  Type *value_;
  bool invalid_;

  static_assert(std::is_same<typename remove_pointer<Type>::type, Type>::value,
                "result type should be non-pointer type");

public:
  explicit result() : value_(nullptr), invalid_(true) {}
  explicit result(Type *value) : value_(value), invalid_(value == nullptr) {}

  result &operator=(Type *rhs) {
    value_ = rhs;
    invalid_ = rhs == nullptr;
    return *this;
  }

  const Type *operator->() const {
    return value_;
  }
  Type *operator->() {
    return value_;
  }

  Type *operator*() {
    return value_;
  }
  const Type *operator*() const {
    return value_;
  }

  operator bool() const {
    return not invalid_ and value_;
  }

  template <typename Derived>
  operator Derived *() const {
    return value_;
  }

  bool valid() const {
    return not invalid_;
  }

  result &invalidate() {
    invalid_ = true;
    return *this;
  }
};
}
}

#endif

