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

#ifndef swift_lexer_location_hh
#define swift_lexer_location_hh

#include <cstddef>
#include <ostream>

namespace swift {
class location {
  const unsigned line_;
  const unsigned column_;

public:
  constexpr location() : line_(0), column_(0) {}

  location(unsigned line, unsigned column) : line_(line), column_(column) {}

  location& operator=(const location& rhs) {
    *const_cast<unsigned*>(&line_) = rhs.line_;
    *const_cast<unsigned*>(&column_) = rhs.column_;
    return *this;
  }

  explicit operator bool() const {
    return line_ or column_;
  }

  unsigned line() const {
    return line_;
  }
  unsigned column() const {
    return column_;
  }

  bool valid() const {
    return static_cast<bool>(*this);
  }
};

class range {
  const location start_;
  const location end_;

public:
  range() = default;
  range(location location) : start_(location), end_(location) {}
  range(location start, location end) : start_(start), end_(end) {}

  range& operator=(const range& rhs) {
    *const_cast<location*>(&start_) = rhs.start_;
    *const_cast<location*>(&end_) = rhs.end_;
    return *this;
  }

  const location& start() const {
    return start_;
  }
  const location& end() const {
    return end_;
  }
};
}

size_t operator-(const swift::location& lhs, const swift::location& rhs);
bool operator==(const swift::location& lhs, const swift::location& rhs);

std::ostream& operator<<(std::ostream&, const swift::location&);
std::ostream& operator<<(std::ostream&, const swift::range&);

#endif

