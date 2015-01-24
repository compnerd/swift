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

#ifndef swift_support_visitor_hh
#define swift_support_visitor_hh

#include <type_traits>

template <typename Type, typename ReturnType, bool ConstVisitor>
struct visits;

template <typename Type, typename ReturnType>
struct visits<Type, ReturnType, /*ConstVisitor*/true> {
  using return_type = ReturnType;
  using reference_type =
      typename std::add_lvalue_reference<typename std::add_const<Type>::type>::type;
  using pointer_type =
      typename std::add_pointer<typename std::add_const<Type>::type>::type;

  virtual ~visits() = default;
  virtual void visit(reference_type) = 0;
};

template <typename Type, typename ReturnType>
struct visits<Type, ReturnType, /*ConstVisitor*/false> {
  using return_type = ReturnType;
  using reference_type =
      typename std::add_lvalue_reference<typename std::remove_const<Type>::type>::type;
  using pointer_type =
      typename std::add_pointer<typename std::remove_const<Type>::type>::type;

  virtual ~visits() = default;
  virtual void visit(reference_type) = 0;
};

template <typename ReturnType = void, bool ConstVisitor = true,
          typename... Types>
class visitor : public visits<Types, ReturnType, ConstVisitor>... {};

template <typename Type, typename ReturnType>
class visitable {
public:
  ReturnType accept(visits<Type, ReturnType, /*ConstVisitor*/ false> &visitor) {
    return visitor.visit(static_cast<Type &>(*this));
  }

  ReturnType
  accept(visits<Type, ReturnType, /*ConstVisitor*/ true> &visitor) const {
    return visitor.visit(static_cast<const Type &>(*this));
  }
};

#endif

