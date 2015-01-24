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

#ifndef swift_semantic_type_hh
#define swift_semantic_type_hh

#include <cstddef>

namespace swift {
namespace ast {
class context;
};

namespace semantic {
#if defined(type_qualifier_support_implemented)
enum class qualifier {
  convenience,
  dynamic,
  final,
  infix,
  lazy,
  mutating,
  nonmutating,
  optional,
  override,
  postfix,
  prefix,
  required,
  static,
  unowned,
  unowned_safe,
  unowned_unsafe,
  weak,
  internal,
  private,
  public,
};
#endif

class type;

class qualified_type {
  const type *type_;
  unsigned qualifiers_;

public:
  qualified_type() = default;

  qualified_type(const type *type, unsigned qualifiers)
      : type_(type), qualifiers_(qualifiers) {}

  const type *type() const {
    return type_;
  }
  bool null() const {
    return type_ == nullptr;
  }

  friend bool operator==(const qualified_type &lhs, const qualified_type &rhs) {
    return lhs.type_ == rhs.type_ and lhs.qualifiers_ == rhs.qualifiers_;
  }
};

enum class typeclass {
  builtin,
};

class type {
  friend class ast::context;

  type(const type &) = delete;
  type &operator=(const type &) = delete;

  class type_info {
    friend class type;
    unsigned type_class : 7;
    /// whether the type is from the AST
    mutable unsigned from_ast : 1;
  };

  qualified_type canonical_type_;

protected:
  class builtin_type_info {
    friend class builtin_type;
    unsigned : 8;
    unsigned builtin_type : 8;
  };

  class function_type_info {
    friend class function_type;
    unsigned : 8;
    unsigned call_info : 8;
  };

  union {
    type_info type_info;
    builtin_type_info builtin_type_info;
  };

  type(typeclass typeclass, qualified_type canonical_type)
      : canonical_type_(canonical_type.null() ? qualified_type(this, 0)
                                              : canonical_type) {
    type_info.type_class = static_cast<unsigned>(typeclass);
    type_info.from_ast = false;
  }

private:
  void from_ast(bool value = true) const {
    type_info.from_ast = value;
  }

public:
  static const constexpr size_t alignment = 16;

  typeclass typeclass() const {
    return static_cast<semantic::typeclass>(type_info.type_class);
  }
  bool from_ast() const {
    return type_info.from_ast;
  }

  bool is_canonical_type_unqualified() const {
    return canonical_type_ == qualified_type(this, 0);
  }

  bool is_literal_type(const ast::context &) const;

  bool is_builtin_type() const;

  bool is_integer_type() const;
  bool is_enumeration_type() const;
  bool is_char_type() const;
  bool is_wchar_type() const;
  bool is_char16_type() const;
  bool is_char32_type() const;
  bool is_integral_type(const ast::context &) const;

  bool is_floating_point_type() const;

  bool is_function_type() const;

  bool is_pointer_type() const;

  template <typename type_>
  const type_ *get_as() const;

  template <typename type_>
  const type_ *cast_as() const;

  void dump() const;
};

class builtin_type : public type {
public:
  enum class kind {
    array,
    boolean,
    character,
    float32,
    float64,
    // float80,
    sint8,
    sint16,
    sint32,
    sint64,
    uint8,
    uint16,
    uint32,
    uint64,
    string,
    tuple,
  };

  builtin_type(kind kind) : type(typeclass::builtin, qualified_type()) {
    builtin_type_info.builtin_type = static_cast<unsigned>(kind);
  }

  builtin_type::kind typeclass() const {
    return static_cast<builtin_type::kind>(builtin_type_info.builtin_type);
  }

  bool is_sugared() const {
    return false;
  }
  qualified_type desugar() const {
    return qualified_type(this, 0);
  }

  bool is_integer() const;
  bool is_signed_integer() const;
  bool is_unsigned_integer() const;

  bool is_floating_point() const;
};
}
}

#endif

