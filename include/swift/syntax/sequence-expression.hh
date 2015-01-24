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

#ifndef swift_syntax_sequence_expression_hh
#define swift_syntax_sequence_expression_hh

#include "swift/syntax/expression.hh"

#include <ext/array_view>

namespace swift::ast {
class context;
class statement;

class sequence_expression : public expression {
  // TODO(compnerd) use placement allocation
  std::vector<expression *> expressions_;

public:
  typedef std::vector<expression *>::const_iterator iterator;
  typedef std::vector<expression *>::const_iterator const_iterator;

  explicit sequence_expression(const std::vector<expression *> &expressions)
      : expression(expression::type::sequence_expression),
        expressions_(expressions) {}

  unsigned size() const {
    return expressions_.size();
  }

  iterator begin() {
    return expressions_.begin();
  }
  iterator end() {
    return expressions_.end();
  }

  const_iterator begin() const {
    return expressions_.begin();
  }
  const_iterator end() const {
    return expressions_.end();
  }
};
}

#endif

