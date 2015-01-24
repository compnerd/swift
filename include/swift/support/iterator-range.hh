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

#ifndef swift_support_iterator_range_hh
#define swift_support_iterator_range_hh

#include <iterator>
#include <utility>

template <typename Iterator>
class iterator_range {
  Iterator begin_, end_;

public:
  // TODO(compnerd) use SFINAE to ensure that the container's iterators match
  // the range's iterator
  template <typename Container>
  iterator_range(Container &&container)
      : begin_(std::begin(container)), end_(std::end(container)) {}

  iterator_range(Iterator begin, Iterator end)
      : begin_(std::move(begin)), end_(std::move(end)) {}

  Iterator begin() const {
    return begin_;
  }
  Iterator end() const {
    return end_;
  }

  size_t size() const {
    return std::distance(end_, begin_);
  }
};

template <typename Iterator>
iterator_range<Iterator> make_range(Iterator begin, Iterator end) {
  return iterator_range<Iterator>(std::move(begin), std::move(end));
}

template <typename Iterator>
iterator_range<Iterator> make_range(std::pair<Iterator, Iterator> range) {
  return iterator_range<Iterator>(std::move(range.first),
                                  std::move(range.second));
}

#endif

