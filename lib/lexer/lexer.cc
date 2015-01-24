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

#include "swift/lexer/lexer.hh"
#include "swift/support/debug.hh"
#include "swift/support/error-handling.hh"

#include <limits>

using namespace swift::diagnostics;

using string = std::char_traits<char32_t>;

namespace {
static constexpr const char32_t *const spelling[] = {
  [static_cast<int>(swift::token::type::l_brace)] = U"{",
  [static_cast<int>(swift::token::type::r_brace)] = U"}",

  [static_cast<int>(swift::token::type::l_paren)] = U"(",
  [static_cast<int>(swift::token::type::r_paren)] = U")",

  [static_cast<int>(swift::token::type::l_square)] = U"[",
  [static_cast<int>(swift::token::type::r_square)] = U"]",

  [static_cast<int>(swift::token::type::amp)] = U"&",
  [static_cast<int>(swift::token::type::at)] = U"@",
  [static_cast<int>(swift::token::type::colon)] = U":",
  [static_cast<int>(swift::token::type::comma)] = U",",
  [static_cast<int>(swift::token::type::equal)] = U"=",
  [static_cast<int>(swift::token::type::exclaim)] = U"!",
  [static_cast<int>(swift::token::type::hash)] = U"#",
  [static_cast<int>(swift::token::type::period)] = U".",
  [static_cast<int>(swift::token::type::question)] = U"?",
  [static_cast<int>(swift::token::type::semi)] = U";",
  [static_cast<int>(swift::token::type::underscore)] = U"_",
  [static_cast<int>(swift::token::type::arrow)] = U"->",

  [static_cast<int>(swift::token::type::kw_as)] = U"as",
  [static_cast<int>(swift::token::type::kw_associativity)] = U"associativity",
  [static_cast<int>(swift::token::type::kw_break)] = U"break",
  [static_cast<int>(swift::token::type::kw_case)] = U"case",
  [static_cast<int>(swift::token::type::kw_class)] = U"class",
  [static_cast<int>(swift::token::type::kw_continue)] = U"continue",
  [static_cast<int>(swift::token::type::kw_convenience)] = U"convenience",
  [static_cast<int>(swift::token::type::kw_default)] = U"default",
  [static_cast<int>(swift::token::type::kw_defer)] = U"defer",
  [static_cast<int>(swift::token::type::kw_deinit)] = U"deinit",
  [static_cast<int>(swift::token::type::kw_didSet)] = U"didSet",
  [static_cast<int>(swift::token::type::kw_do)] = U"do",
  [static_cast<int>(swift::token::type::kw_dynamic)] = U"dynamic",
  [static_cast<int>(swift::token::type::kw_dynamicType)] = U"dynamicType",
  [static_cast<int>(swift::token::type::kw_else)] = U"else",
  [static_cast<int>(swift::token::type::kw_enum)] = U"enum",
  [static_cast<int>(swift::token::type::kw_extension)] = U"extension",
  [static_cast<int>(swift::token::type::kw_fallthrough)] = U"fallthrough",
  [static_cast<int>(swift::token::type::kw_final)] = U"final",
  [static_cast<int>(swift::token::type::kw_for)] = U"for",
  [static_cast<int>(swift::token::type::kw_func)] = U"func",
  [static_cast<int>(swift::token::type::kw_get)] = U"get",
  [static_cast<int>(swift::token::type::kw_guard)] = U"guard",
  [static_cast<int>(swift::token::type::kw_if)] = U"if",
  [static_cast<int>(swift::token::type::kw_import)] = U"import",
  [static_cast<int>(swift::token::type::kw_in)] = U"in",
  [static_cast<int>(swift::token::type::kw_infix)] = U"infix",
  [static_cast<int>(swift::token::type::kw_init)] = U"init",
  [static_cast<int>(swift::token::type::kw_inout)] = U"inout",
  [static_cast<int>(swift::token::type::kw_internal)] = U"internal",
  [static_cast<int>(swift::token::type::kw_is)] = U"is",
  [static_cast<int>(swift::token::type::kw_lazy)] = U"lazy",
  [static_cast<int>(swift::token::type::kw_left)] = U"left",
  [static_cast<int>(swift::token::type::kw_let)] = U"let",
  [static_cast<int>(swift::token::type::kw_mutating)] = U"mutating",
  [static_cast<int>(swift::token::type::kw_none)] = U"none",
  [static_cast<int>(swift::token::type::kw_nonmutating)] = U"nonmutating",
  [static_cast<int>(swift::token::type::kw_operator)] = U"operator",
  [static_cast<int>(swift::token::type::kw_optional)] = U"optional",
  [static_cast<int>(swift::token::type::kw_override)] = U"override",
  [static_cast<int>(swift::token::type::kw_postfix)] = U"postfix",
  [static_cast<int>(swift::token::type::kw_precedence)] = U"precedence",
  [static_cast<int>(swift::token::type::kw_prefix)] = U"prefix",
  [static_cast<int>(swift::token::type::kw_private)] = U"private",
  [static_cast<int>(swift::token::type::kw_protocol)] = U"protocol",
  [static_cast<int>(swift::token::type::kw_public)] = U"public",
  [static_cast<int>(swift::token::type::kw_required)] = U"required",
  [static_cast<int>(swift::token::type::kw_repeat)] = U"repeat",
  [static_cast<int>(swift::token::type::kw_rethrows)] = U"rethrows",
  [static_cast<int>(swift::token::type::kw_return)] = U"return",
  [static_cast<int>(swift::token::type::kw_right)] = U"right",
  [static_cast<int>(swift::token::type::kw_self)] = U"self",
  [static_cast<int>(swift::token::type::kw_set)] = U"set",
  [static_cast<int>(swift::token::type::kw_static)] = U"static",
  [static_cast<int>(swift::token::type::kw_struct)] = U"struct",
  [static_cast<int>(swift::token::type::kw_subscript)] = U"subscript",
  [static_cast<int>(swift::token::type::kw_super)] = U"super",
  [static_cast<int>(swift::token::type::kw_switch)] = U"switch",
  [static_cast<int>(swift::token::type::kw_try)] = U"try",
  [static_cast<int>(swift::token::type::kw_typealias)] = U"typealias",
  [static_cast<int>(swift::token::type::kw_unowned)] = U"unowned",
  [static_cast<int>(swift::token::type::kw_var)] = U"var",
  [static_cast<int>(swift::token::type::kw_weak)] = U"weak",
  [static_cast<int>(swift::token::type::kw_where)] = U"where",
  [static_cast<int>(swift::token::type::kw_while)] = U"while",
  [static_cast<int>(swift::token::type::kw_willSet)] = U"willSet",

  [static_cast<int>(swift::token::type::kw_Protocol)] = U"Protocol",
  [static_cast<int>(swift::token::type::kw_Self)] = U"Self",
  [static_cast<int>(swift::token::type::kw_Type)] = U"Type",

  [static_cast<int>(swift::token::type::kw___COLUMN__)] = U"__COLUMN__",
  [static_cast<int>(swift::token::type::kw___FILE__)] = U"__FILE__",
  [static_cast<int>(swift::token::type::kw___FUNCTION__)] = U"__FUNCTION__",
  [static_cast<int>(swift::token::type::kw___LINE__)] = U"__LINE__",

  [static_cast<int>(swift::token::type::pp_available)] = U"#available",
  [static_cast<int>(swift::token::type::pp_if)] = U"#if",
  [static_cast<int>(swift::token::type::pp_else)] = U"#else",
  [static_cast<int>(swift::token::type::pp_elseif)] = U"#elseif",
  [static_cast<int>(swift::token::type::pp_endif)] = U"#endif",
  [static_cast<int>(swift::token::type::pp_line)] = U"#line",
};

template <char32_t Begin, char32_t End>
struct range {
  static constexpr bool contains(char32_t ch) {
    return ch >= Begin and ch <= End;
  }
};

template <char32_t...>
struct set;

template <>
struct set<> {
  static constexpr bool contains(char32_t) {
    return false;
  }
};

template <char32_t Value, char32_t... Values>
struct set<Value, Values...> {
  static constexpr bool contains(char32_t ch) {
    return ch == Value or set<Values...>::contains(ch);
  }
};

// identifier-head → Upper or lowercase letter A through Z
// identifier-head → _
// identifier-head → U+00A8, U+00AA, U+00AD, U+00AF, U+00B2–U+00B5, or U+00B7–U+00BA
// identifier-head → U+00BC–U+00BE, U+00C0–U+00D6, U+00D8–U+00F6, or U+00F8–U+00FF
// identifier-head → U+0100–U+02FF, U+0370–U+167F, U+1681–U+180D, or U+180F–U+1DBF
// identifier-head → U+1E00–U+1FFF
// identifier-head → U+200B–U+200D, U+202A–U+202E, U+203F–U+2040, U+2054, or U+2060–U+206F
// identifier-head → U+2070–U+20CF, U+2100–U+218F, U+2460–U+24FF, or U+2776–U+2793
// identifier-head → U+2C00–U+2DFF or U+2E80–U+2FFF
// identifier-head → U+3004–U+3007, U+3021–U+302F, U+3031–U+303F, or U+3040–U+D7FF
// identifier-head → U+F900–U+FD3D, U+FD40–U+FDCF, U+FDF0–U+FE1F, or U+FE30–U+FE44
// identifier-head → U+FE47–U+FFFD
// identifier-head → U+10000–U+1FFFD, U+20000–U+2FFFD, U+30000–U+3FFFD, or U+40000–U+4FFFD
// identifier-head → U+50000–U+5FFFD, U+60000–U+6FFFD, U+70000–U+7FFFD, or U+80000–U+8FFFD
// identifier-head → U+90000–U+9FFFD, U+A0000–U+AFFFD, U+B0000–U+BFFFD, or U+C0000–U+CFFFD
// identifier-head → U+D0000–U+DFFFD or U+E0000–U+EFFFD
bool is_identifier_head(char32_t ch) {
  return isalpha(ch) ||
         set<U'_', U'\u00a8', U'\u00aa', U'\u00ad', U'\u00af'>::contains(ch) ||
         range<U'\u00b2', U'\u00b5'>::contains(ch) ||
         range<U'\u00b7', U'\u00ba'>::contains(ch) ||
         range<U'\u00bc', U'\u00be'>::contains(ch) ||
         range<U'\u00c0', U'\u00d6'>::contains(ch) ||
         range<U'\u00d8', U'\u00f6'>::contains(ch) ||
         range<U'\u00f8', U'\u00ff'>::contains(ch) ||
         range<U'\u0100', U'\u02ff'>::contains(ch) ||
         range<U'\u0370', U'\u167f'>::contains(ch) ||
         range<U'\u1681', U'\u180d'>::contains(ch) ||
         range<U'\u180f', U'\u1dbf'>::contains(ch) ||
         range<U'\u1e00', U'\u1fff'>::contains(ch) ||
         range<U'\u200b', U'\u200d'>::contains(ch) ||
         range<U'\u202a', U'\u202e'>::contains(ch) ||
         range<U'\u203f', U'\u2040'>::contains(ch) ||
         set<U'\u2054'>::contains(ch) ||
         range<U'\u2060', U'\u206f'>::contains(ch) ||
         range<U'\u2070', U'\u20cf'>::contains(ch) ||
         range<U'\u2100', U'\u218f'>::contains(ch) ||
         range<U'\u2460', U'\u24ff'>::contains(ch) ||
         range<U'\u2776', U'\u2793'>::contains(ch) ||
         range<U'\u2c00', U'\u2dff'>::contains(ch) ||
         range<U'\u2e80', U'\u2fff'>::contains(ch) ||
         range<U'\u3004', U'\u3007'>::contains(ch) ||
         range<U'\u3021', U'\u302f'>::contains(ch) ||
         range<U'\u3031', U'\u303f'>::contains(ch) ||
         range<U'\u3040', U'\ud7ff'>::contains(ch) ||
         range<U'\uf900', U'\ufd3d'>::contains(ch) ||
         range<U'\ufd40', U'\ufdcf'>::contains(ch) ||
         range<U'\ufdf0', U'\ufe1f'>::contains(ch) ||
         range<U'\ufe30', U'\ufe44'>::contains(ch) ||
         range<U'\ufe47', U'\ufffd'>::contains(ch) ||
         range<U'\U00010000', U'\U0001fffd'>::contains(ch) ||
         range<U'\U00020000', U'\U0002fffd'>::contains(ch) ||
         range<U'\U00030000', U'\U0003fffd'>::contains(ch) ||
         range<U'\U00040000', U'\U0004fffd'>::contains(ch) ||
         range<U'\U00050000', U'\U0005fffd'>::contains(ch) ||
         range<U'\U00060000', U'\U0006fffd'>::contains(ch) ||
         range<U'\U00070000', U'\U0007fffd'>::contains(ch) ||
         range<U'\U00080000', U'\U0008fffd'>::contains(ch) ||
         range<U'\U00090000', U'\U0009fffd'>::contains(ch) ||
         range<U'\U000a0000', U'\U000afffd'>::contains(ch) ||
         range<U'\U000b0000', U'\U000bfffd'>::contains(ch) ||
         range<U'\U000c0000', U'\U000cfffd'>::contains(ch) ||
         range<U'\U000d0000', U'\U000dfffd'>::contains(ch) ||
         range<U'\U000e0000', U'\U000efffd'>::contains(ch);
}

// identifier-character → Digit 0 through 9
// identifier-character → U+0300–U+036F, U+1DC0–U+1DFF, U+20D0–U+20FF, or U+FE20–U+FE2F
// identifier-character → identifier-head
bool is_identifier_character(char32_t ch) {
  return isdigit(ch) || range<U'\u0300', U'\u036f'>::contains(ch) ||
         range<U'\u1dc0', U'\u1dff'>::contains(ch) ||
         range<U'\u20d0', U'\u20ff'>::contains(ch) ||
         range<U'\ufe20', U'\uf32f'>::contains(ch) || is_identifier_head(ch);
}

// operator-head → / | = | - | + | ! | * | % | < | > | & | | | ^ | ~
// operator-head → U+00A1–U+00A7
// operator-head → U+00A9 or U+00AB
// operator-head → U+00AC or U+00AE
// operator-head → U+00B0–U+00B1, U+00B6, U+00BB, U+00BF, U+00D7, or U+00F7
// operator-head → U+2016–U+2017 or U+2020–U+2027
// operator-head → U+2030–U+203E
// operator-head → U+2041–U+2053
// operator-head → U+2055–U+205E
// operator-head → U+2190–U+23FF
// operator-head → U+2500–U+2775
// operator-head → U+2794–U+2BFF
// operator-head → U+2E00–U+2E7F
// operator-head → U+3001–U+3003
// operator-head → U+3008–U+3030
bool is_operator_head(char32_t ch) {
  return set<U'/', U'=', U'-', U'+', U'!', U'*', U'%', U'<', U'>', U'&', U'|',
             U'^', U'~'>::contains(ch) ||
         range<U'\u00a1', U'\u00a7'>::contains(ch) ||
         set<U'\u00a9', U'\u00ab', U'\u00ac', U'\u00ae'>::contains(ch) ||
         set<U'\u00b0', U'\u00b1', U'\u00b6', U'\u00bb', U'\u00bf', U'\u00d7',
             U'\u00f7'>::contains(ch) ||
         range<U'\u2016', U'\u2017'>::contains(ch) ||
         range<U'\u2020', U'\u2027'>::contains(ch) ||
         range<U'\u2030', U'\u203e'>::contains(ch) ||
         range<U'\u2041', U'\u2053'>::contains(ch) ||
         range<U'\u2055', U'\u205e'>::contains(ch) ||
         range<U'\u2190', U'\u23ff'>::contains(ch) ||
         range<U'\u2500', U'\u2775'>::contains(ch) ||
         range<U'\u2794', U'\u2bff'>::contains(ch) ||
         range<U'\u2e00', U'\u2e7f'>::contains(ch) ||
         range<U'\u3001', U'\u3003'>::contains(ch) ||
         range<U'\u3008', U'\u3030'>::contains(ch);
}

// operator-character → operator-head
// operator-character → U+0300–U+036F
// operator-character → U+1DC0–U+1DFF
// operator-character → U+20D0–U+20FF
// operator-character → U+FE00–U+FE0F
// operator-character → U+FE20–U+FE2F
// operator-character → U+E0100–U+E01EF
bool is_operator_character(char32_t ch) {
  return is_operator_head(ch) || range<U'\u0300', U'\u036f'>::contains(ch) ||
         range<U'\u1dc0', U'\u1dff'>::contains(ch) ||
         range<U'\u2000', U'\u20ff'>::contains(ch) ||
         range<U'\ufe00', U'\ufe0f'>::contains(ch) ||
         range<U'\ufe20', U'\ufe2f'>::contains(ch) ||
         range<U'\U000e0100', U'\U000e01ef'>::contains(ch);
}

// dot-operator-character → '.' | operator-character
bool is_dot_operator_character(char32_t ch) {
  return set<U'.'>::contains(ch) or is_operator_character(ch);
}

// dot-operator-head → '..'
bool is_dot_operator_head(const char32_t ch[2]) {
  return ch[0] == U'.' and ch[1] == U'.';
}

static bool isbdigit(char32_t ch) {
  return range<U'0', U'1'>::contains(ch);
}

static bool isodigit(char32_t ch) {
  return range<U'0', U'7'>::contains(ch);
}
}

namespace swift {
// TODO(compnerd) move this out of the lexer once the identifier_table is setup
const char32_t *token::canonical_spelling(token::type token_type) {
  DEBUG_ONLY({
    switch (token_type) {
    case token::type::invalid:
    case token::type::eof:
    case token::type::whitespace:
    case token::type::comment:
    case token::type::identifier:
    case token::type::literal:
    case token::type::op:
      assert(not (token_type == token_type) &&
             "identifiers have no canonical spelling");
      break;
    default:
      break;
    }
  });
  return spelling[static_cast<int>(token_type)];
}

template <token::type Type>
token lexer::consume() {
  const char32_t *lexeme = cursor_;

  auto b = position();
  cursor_ = cursor_ + string::length(spelling[static_cast<int>(Type)]);
  column_ = column_ + string::length(spelling[static_cast<int>(Type)]);
  auto e = position();

  return std::move<token>({
      Type, std::u32string_view(lexeme, e.column() - b.column()), b, e
  });
}

template <>
token lexer::consume<token::type::whitespace>() {
  auto b = position();
  while (cursor_ < buffer_end_)
    switch (*cursor_) {
    default:
      return std::move<token>({
          token::type::whitespace, std::u32string_view(), b, position()
      });
    case U'\0':
    case U' ':
    case U'\t':
      column_++;
      cursor_++;
      break;
    case U'\f':
      cursor_++;
      break;
    case U'\r':
    case U'\n':
    case U'\v':
      cursor_++;
      column_ = 0;
      line_++;
      break;
    }
  return std::move<token>({});
}

template <>
token lexer::consume<token::type::comment>() {
  if (buffer_end_ - cursor_ < 2)
    __builtin_trap();  // TODO(compnerd) diagnose unexpected end

  assert(cursor_[0] == U'/' && (cursor_[1] == U'/' || cursor_[1] == U'*') &&
         "comment must start with '//' or '/*'");

  auto b = position();
  if (cursor_[0] == U'/' && cursor_[1] == U'/') {
    while (cursor_ < buffer_end_ && (*cursor_ != U'\r' && *cursor_ != U'\n'))
      ++cursor_, ++column_;
    return std::move<token>({
        token::type::comment, std::u32string_view(), b, position()
    });
  }

  assert(cursor_[0] == U'/' && cursor_[1] == U'*' &&
         "expected '/*' comment marker");

  int depth = 1;
  cursor_ = cursor_ + 2, column_ = column_ + 2;
  while (depth && buffer_end_ - cursor_ >= 2)
    if (cursor_[0] == U'/' && cursor_[1] == U'*')
      ++depth, cursor_ = cursor_ + 2, column_ = column_ + 2;
    else if (cursor_[0] == U'*' && cursor_[1] == U'/')
      --depth, cursor_ = cursor_ + 2, column_ = column_ + 2;
    else if (cursor_[0] == U'/' && cursor_[1] == U'/')
      while (cursor_ < buffer_end_ && *cursor_ != U'\r' && *cursor_ != U'\n')
        ++cursor_, ++column_;
    else
      ++cursor_, ++column_;

  if (depth)
    __builtin_trap();  // TODO(compnerd) diagnose unexpected end

  return std::move<token>({
      token::type::comment, std::u32string_view(), b, position()
  });
}

template <>
token lexer::consume<token::type::identifier>() {
  const char32_t *name = cursor_;
  bool reserved = false;

  if (not (*cursor_ == U'$' or *cursor_ == U'`' or is_identifier_head(*cursor_))) {
    diagnostics_engine_.report(position(),
                               diagnostic::err_invalid_character_in_source_file);
    return std::move<token>({ token::type::invalid, std::u32string_view(),
                              position(), position() });
  }

  auto b = position();
  switch (*cursor_) {
  case U'$':
    do
      ++cursor_, ++column_;
    while (isdigit(*cursor_));
    break;
  case U'`':
    reserved = true;
    ++cursor_, ++column_;
    /* fall through */
  default:
    if (!is_identifier_head(*cursor_))
      __builtin_trap();  // TODO(compnerd) diagnose invalid identifier

    while (is_identifier_character(*cursor_))
      ++cursor_, ++column_;

    if (!reserved)
      break;

    if (*cursor_ == U'`')
      ++cursor_, ++column_;
    else
      __builtin_trap();  // TODO(compnerd) diagnose missing `

    break;
  }
  auto e = position();

  return std::move<token>({
      std::u32string_view(name, e.column() - b.column()), b, e
  });
}

// quoted-text-item → escaped-character
// quoted-text-item → '\' '(' expression ')'
// quoted-text-item → Any Unicode extended grapheme cluster except " , \ , U+000A, or U+000D
//
// escaped-character → '\0'
// escaped-character → '\\'
// escaped-character → '\t'
// escaped-character → '\n'
// escaped-character → '\r'
// escaped-character → '\"'
// escaped-character → '\''
// escaped-character → '\' 'u' '{' unicode-scalar-digits '}'
//
// unicode-scalar-digits → Between one and eight hexadecimal digits
template <>
token lexer::consume<token::literal_type::string>() {
  assert((buffer_end_ - cursor_ >= 2 and *cursor_ == U'"') &&
         "expected string-literal");

  const char32_t *literal = cursor_;

  auto b = position();
  for (++cursor_, ++column_;
       not set<U'"', U'\u000a', U'\u000d'>::contains(*cursor_);
       ++cursor_, ++column_) {
    if (*cursor_ == U'\\') {
      switch (cursor_[1]) {
      default:
        __builtin_trap();  // TODO(compnerd) diagnose invalid string-literal
      case U'0':
      case U'\\':
      case U't':
      case U'n':
      case U'r':
      case U'"':
      case U'\'':
        ++cursor_, ++column_;  // '\'
        ++cursor_, ++column_;  // .
        break;
      case 'u':
        __builtin_trap();  // FIXME(compnerd) handle unicode codepoint
      case '(':
        ++cursor_, ++column_;  // '\'
        ++cursor_, ++column_;  // '('
        for (unsigned level = 1; level > 0; ++cursor_, ++column_) {
          switch (*cursor_) {
          default:
            break;
          case '(':
            assert(level < std::numeric_limits<unsigned>::max() && "overflow");
            ++level;
            break;
          case ')':
            --level;
            break;
          }
        }
        break;
      }

      // NOTE(compnerd) reset cursor and column to be increment in the loop
      --cursor_, --column_;
    }
  }
  assert(*cursor_ == U'"' && "expected '\"'");
  ++cursor_, ++column_;
  auto e = position();

  return std::move<token>({
      token::literal_type::string,
      std::u32string_view(literal + 1, e.column() - b.column() - 2), b, e
  });
}

template <>
token lexer::consume<token::literal_type::constant_true>() {
  assert((buffer_end_ - cursor_ >= 4 and
          string::compare(cursor_, U"true", 4) == 0) &&
         "expected 'true'");

  auto b = position();
  cursor_ = cursor_ + string::length(U"true");
  column_ = column_ + string::length(U"true");
  auto e = position();

  return std::move<token>({
      token::literal_type::constant_true, U"true", b, e
  });
}

template <>
token lexer::consume<token::literal_type::constant_false>() {
  assert((buffer_end_ - cursor_ >= 5 and
          string::compare(cursor_, U"false", 5) == 0) &&
         "expected 'false'");

  auto b = position();
  cursor_ = cursor_ + string::length(U"false");
  column_ = column_ + string::length(U"false");
  auto e = position();

  return std::move<token>({
      token::literal_type::constant_false, U"false", b, e
  });
}

template <>
token lexer::consume<token::literal_type::constant_nil>() {
  assert((buffer_end_ - cursor_ >= 3 and
          string::compare(cursor_, U"nil", 3) == 0) &&
         "expected 'nil'");

  auto b = position();
  cursor_ = cursor_ + string::length(U"nil");
  column_ = column_ + string::length(U"nil");
  auto e = position();

  return std::move<token>({ token::literal_type::constant_nil, U"nil", b, e });
}

template <>
token lexer::consume<token::type::literal>() {
  assert(buffer_end_ - cursor_ >= 1 && "cannot consume from an empty stream");

  switch (*cursor_) {
  default:
    swift_unreachable("attempted to lex non-literal as literal");
  case U'f':  // false
    return std::move<token>(consume<token::literal_type::constant_false>());
  case U'n':  // nil
    return std::move<token>(consume<token::literal_type::constant_nil>());
  case U't':  // true
    return std::move<token>(consume<token::literal_type::constant_true>());
  case U'"':  // string-literal
    return std::move<token>(consume<token::literal_type::string>());
  case U'-':
  case U'0' ... U'9': {  // integer-literal | floating-point-literal
    token::literal_type type = token::literal_type::integral;
    auto b = position();

    const char32_t *literal = cursor_;

    if (cursor_[0] == U'-')
      ++cursor_, ++column_;

    if (*cursor_ == U'0' and set<U'b', U'o', U'x'>::contains(cursor_[1])) {
      ++cursor_, ++column_;
      if (cursor_ < buffer_end_)
        switch (*cursor_) {
        case U'b':
          assert(buffer_end_ - cursor_ >= 2 &&
                 "expected a digit after literal prefix");
          for (++cursor_, ++column_; isbdigit(*cursor_) or *cursor_ == U'_';
               ++cursor_, ++column_)
            ;
          break;
        case U'o':
          assert(buffer_end_ - cursor_ >= 2 &&
                 "expected a digit after literal prefix");
          for (++cursor_, ++column_; isodigit(*cursor_) or *cursor_ == U'_';
               ++cursor_, ++column_)
            ;
          break;
        case U'x':
          assert(buffer_end_ - cursor_ >= 2 &&
                 "expected a digit after literal prefix");
          for (++cursor_, ++column_; isxdigit(*cursor_) or *cursor_ == U'_';
               ++cursor_, ++column_)
            ;

          if (*cursor_ == '.') {
            type = token::literal_type::floating_point;
            __builtin_trap();  // TODO(compnerd) generate a floating point literal
          }

          break;
        }
    } else {
      while (isdigit(*cursor_) || *cursor_ == '_')
        ++cursor_, ++column_;

      if (*cursor_ == '.' and isdigit(cursor_[1])) {
        type = token::literal_type::floating_point;
        do
          ++cursor_, ++column_;
        while (isxdigit(*cursor_) || *cursor_ == U'_');
      }
    }

    auto e = position();

    return std::move<token>({
        type, std::u32string_view(literal, e.column() - b.column()), b, e
    });
  }
  }
}

// operator → operator-head operator-characters[opt]
// operator → dot-operator-head dot-operator-characters[opt]
template <>
token lexer::consume<token::type::op>() {
  assert(is_operator_head(*cursor_) or is_dot_operator_head(cursor_));

  using whitespace = set<U'\0', U' ', U'\t', U'\f', U'\r', U'\n', U'\v'>;

  /// For the purposes of these rules, the characters '(', '[', and '{' before
  /// an operator, the characters ')', ']', and '}' after an operator, and the
  /// characters ',', ';', and ':' are also considered whitespace.
  using left_ws_chars = set<U'\0', U' ', U'\t', U'\f', U'\r', U'\n', U'\v',
                            U',', U';', U':', U'(', U'[', U'{'>;
  using right_ws_chars = set<U'\0', U' ', U'\t', U'\f', U'\r', U'\n', U'\v',
                             U',', U';', U':', U')', U']', U'}'>;

  const char32_t *op = cursor_;
  bool dotted = is_dot_operator_head(cursor_);

  auto b = position();
  do
    ++cursor_, ++column_;
  while (dotted ? is_dot_operator_character(*cursor_)
                : is_operator_character(*cursor_));
  auto e = position();

  /// If an operator has whitespace around both sides or around neither side, it
  /// is treated as a binary operator.
  ///
  /// If an operator has whitespace on the left side only, it is treated as a
  /// prefix unary operator.
  ///
  /// If an operator has whitespace on the right side only, it is treated as a
  /// postfix unary operator.
  ///
  /// If an operator has no whitespace on the left but is followed immediately
  /// by a dot (.), it is treated as a postfix unary operator.
  token::operator_type type = token::operator_type::invalid;
  bool ws_left = op == buffer_start_ or left_ws_chars::contains(op[-1]);
  bool ws_right = right_ws_chars::contains(*cursor_);
  bool period_follows = false;
  {
    const char32_t *ptr = cursor_;
    while (ptr < buffer_end_ and whitespace::contains(*ptr))
      ++ptr;
    period_follows = ptr < buffer_end_ and *ptr == U'.';
  }
  if ((ws_left and ws_right) or
      (not ws_left and not ws_right and not period_follows)) {
    type = token::operator_type::binary;
  } else if (ws_left and not ws_right) {
    type = token::operator_type::unary_prefix;
  } else if (not ws_left and (ws_right or period_follows)) {
    type = token::operator_type::unary_postfix;
  }

  return std::move<token>({
      type, std::u32string_view(op, e.column() - b.column()), b, e
  });
}

template <>
token lexer::consume<token::type::question>() {
  using whitespace = set<U'\0', U' ', U'\t', U'\f', U'\r', U'\n', U'\v'>;

  const char32_t *lexeme = cursor_;

  auto b = position();
  cursor_ = cursor_ +
            string::length(spelling[static_cast<int>(token::type::question)]);
  column_ = column_ +
            string::length(spelling[static_cast<int>(token::type::question)]);
  auto e = position();

  /// To use the '?' as the optional-chaining operator, it must not have
  /// whitespace on the left. To use it in the ternary conditional ('?' ':')
  /// operator, it must have whitespace around both sides.
  token::operator_type operator_type = token::operator_type::invalid;
  bool ws_left = cursor_ == buffer_start_ or whitespace::contains(lexeme[-1]);
  bool ws_right = cursor_ == buffer_end_ or whitespace::contains(*cursor_);
  if (not ws_left)
    operator_type = token::operator_type::unary_postfix;
  else if (ws_left and ws_right)
    operator_type = token::operator_type::binary;

  // XXX(compnerd) should the use of the '?' as an optional-chaining operator
  // or a ternary operator be tracked differently?
  return std::move<token>({
      token::type::question, operator_type,
      std::u32string_view(lexeme, e.column() - b.column()), b, e
  });
}

template <>
token lexer::consume<token::type::exclaim>() {
  using whitespace = set<U'\0', U' ', U'\t', U'\f', U'\r', U'\n', U'\v'>;

  const char32_t *lexeme = cursor_;

  auto b = position();
  cursor_ = cursor_ +
            string::length(spelling[static_cast<int>(token::type::exclaim)]);
  column_ = column_ +
            string::length(spelling[static_cast<int>(token::type::exclaim)]);
  auto e = position();

  // If the ! or ? predefined operator has no whitespace on the left, it is
  // treated as a postfix operator, regardless of whether it has whitespace on
  // the right.
  token::operator_type operator_type = token::operator_type::invalid;
  bool ws_left = cursor_ == buffer_start_ or whitespace::contains(lexeme[-1]);
  bool ws_right = cursor_ == buffer_end_ or whitespace::contains(*cursor_);
  if (not ws_left)
    operator_type = (lexeme == buffer_start_)
                        ? token::operator_type::unary_prefix
                        : token::operator_type::unary_postfix;
  if (ws_left and ws_right)
    operator_type = token::operator_type::binary;
  else if (ws_left)
    operator_type = token::operator_type::unary_prefix;

  return std::move<token>({
      token::type::exclaim, operator_type,
      std::u32string_view(lexeme, e.column() - b.column()), b, e
  });
}

template <token::type Type>
bool lexer::match() const {
  auto constexpr token_spelling = spelling[static_cast<int>(Type)];
  const auto token_length = string::length(token_spelling);
  return static_cast<size_t>(buffer_end_ - cursor_) >= token_length and
         std::u32string_view(cursor_, token_length) == token_spelling and
         (static_cast<size_t>(buffer_end_ - cursor_) == token_length or
          (isalpha(token_spelling[0])
               ? not is_identifier_character(cursor_[token_length])
               : not isalpha(cursor_[token_length])));
}

template <>
bool lexer::match<token::type::literal>() const {
  return (buffer_end_ - cursor_ >= 1 &&
          (isdigit(*cursor_) || *cursor_ == U'"')) ||
         (buffer_end_ - cursor_ >= 2 && cursor_[0] == U'-' &&
          isdigit(cursor_[1])) ||
         (buffer_end_ - cursor_ >= 3 &&
          string::compare(cursor_, U"nil", 3) == 0) ||
         (buffer_end_ - cursor_ >= 4 &&
          string::compare(cursor_, U"true", 4) == 0) ||
         (buffer_end_ - cursor_ >= 5 &&
          string::compare(cursor_, U"false", 5) == 0);
}

// operator → operator-head operator-characters[opt]
// operator → dot-operator-head dot-operator-characters[opt]
// dot-operator-head → '..'
template <>
bool lexer::match<token::type::op>() const {
  if (cursor_[0] == U'/' and (cursor_[1] == U'/' or cursor_[1] == '*'))
    return false;
  if (cursor_[0] == U'-' and isdigit(cursor_[1]))
    return false;
  return is_operator_head(*cursor_) or is_dot_operator_head(cursor_);
}

token lexer::lex() {
  assert(cursor_ <= buffer_end_ && "cursor may not extend beyond buffer_end_");
  if (cursor_ == buffer_end_)
    return token();

  consume<token::type::whitespace>();
  if (cursor_ == buffer_end_)
    return token();

  // FIXME(compnerd) detect ambiguities between custom operator and builtin
  // operators better -- alternatively, update parser to treat '=' as
  // operators rather than primitive tokens
  if (match<token::type::op>() and
      not (buffer_end_ - cursor_ == 1 and set<U'=', U'!'>::contains(cursor_[0])) and
      not (buffer_end_ - cursor_ > 1 and set<U'=', U'!'>::contains(cursor_[0]) and
           not is_operator_character(cursor_[1])) and
      not (buffer_end_ - cursor_ > 2 and cursor_[0] == U'-' and
           cursor_[1] == U'>' and not is_operator_character(cursor_[2])))
    return consume<token::type::op>();

  switch (*cursor_) {
  case U'{':
    return consume<token::type::l_brace>();
  case U'}':
    return consume<token::type::r_brace>();
  case U'(':
    return consume<token::type::l_paren>();
  case U')':
    return consume<token::type::r_paren>();
  case U'[':
    return consume<token::type::l_square>();
  case U']':
    return consume<token::type::r_square>();
  case U'&':
    return consume<token::type::amp>();
  case U'@':
    return consume<token::type::at>();
  case U':':
    return consume<token::type::colon>();
  case U',':
    return consume<token::type::comma>();
  case U'=':
    return consume<token::type::equal>();
  case U'!':
    return consume<token::type::exclaim>();
  case U'#':
    if (match<token::type::pp_available>())
      return consume<token::type::pp_available>();
    if (match<token::type::pp_if>())
      return consume<token::type::pp_if>();
    if (match<token::type::pp_else>())
      return consume<token::type::pp_else>();
    if (match<token::type::pp_elseif>())
      return consume<token::type::pp_elseif>();
    if (match<token::type::pp_endif>())
      return consume<token::type::pp_endif>();
    if (match<token::type::pp_line>())
      return consume<token::type::pp_line>();
    return consume<token::type::hash>();
  case U'-':
    if (isdigit(cursor_[1]))
      break;
    if (match<token::type::arrow>())
      return consume<token::type::arrow>();
    break;
  case U'.':
    return consume<token::type::period>();
  case U'?':
    return consume<token::type::question>();
  case U';':
    return consume<token::type::semi>();
  case U'/':
    if (cursor_[1] == U'/' || cursor_[1] == U'*')
      return consume<token::type::comment>(), next();
    break;
  case U'_':
    if (match <token::type::kw___COLUMN__>())
      return consume<token::type::kw___COLUMN__>();
    if (match<token::type::kw___FILE__>())
      return consume<token::type::kw___FILE__>();
    if (match<token::type::kw___FUNCTION__>())
      return consume<token::type::kw___FUNCTION__>();
    if (match<token::type::kw___LINE__>())
      return consume<token::type::kw___LINE__>();
    if (buffer_end_ - cursor_ > 1 && !is_identifier_character(*cursor_))
      return consume<token::type::underscore>();
    break;
  case U'a':
    if (match<token::type::kw_associativity>())
      return consume<token::type::kw_associativity>();
    if (match<token::type::kw_as>())
      return consume<token::type::kw_as>();
    break;
  case U'b':
    if (match<token::type::kw_break>())
      return consume<token::type::kw_break>();
    break;
  case U'c':
    if (match<token::type::kw_case>())
      return consume<token::type::kw_case>();
    if (match<token::type::kw_class>())
      return consume<token::type::kw_class>();
    if (match<token::type::kw_continue>())
      return consume<token::type::kw_continue>();
    if (match<token::type::kw_convenience>())
      return consume<token::type::kw_convenience>();
    break;
  case U'd':
    if (match<token::type::kw_default>())
      return consume<token::type::kw_default>();
    if (match<token::type::kw_defer>())
      return consume<token::type::kw_defer>();
    if (match<token::type::kw_deinit>())
      return consume<token::type::kw_deinit>();
    if (match<token::type::kw_didSet>())
      return consume<token::type::kw_didSet>();
    if (match<token::type::kw_do>())
      return consume<token::type::kw_do>();
    if (match<token::type::kw_dynamicType>())
      return consume<token::type::kw_dynamicType>();
    if (match<token::type::kw_dynamic>())
      return consume<token::type::kw_dynamic>();
    break;
  case U'e':
    if (match<token::type::kw_else>())
      return consume<token::type::kw_else>();
    if (match<token::type::kw_enum>())
      return consume<token::type::kw_enum>();
    if (match<token::type::kw_extension>())
      return consume<token::type::kw_extension>();
    break;
  case U'f':
    if (match<token::type::kw_fallthrough>())
      return consume<token::type::kw_fallthrough>();
    if (match<token::type::kw_final>())
      return consume<token::type::kw_final>();
    if (match<token::type::kw_for>())
      return consume<token::type::kw_for>();
    if (match<token::type::kw_func>())
      return consume<token::type::kw_func>();
    break;
  case U'g':
    if (match<token::type::kw_get>())
      return consume<token::type::kw_get>();
    if (match<token::type::kw_guard>())
      return consume<token::type::kw_guard>();
    break;
  case U'h':
    break;
  case U'i':
    if (match<token::type::kw_if>())
      return consume<token::type::kw_if>();
    if (match<token::type::kw_infix>())
      return consume<token::type::kw_infix>();
    if (match<token::type::kw_import>())
      return consume<token::type::kw_import>();
    if (match<token::type::kw_init>())
      return consume<token::type::kw_init>();
    if (match<token::type::kw_inout>())
      return consume<token::type::kw_inout>();
    if (match<token::type::kw_internal>())
      return consume<token::type::kw_internal>();
    if (match<token::type::kw_in>())
      return consume<token::type::kw_in>();
    if (match<token::type::kw_is>())
      return consume<token::type::kw_is>();
    break;
  case U'j':
    break;
  case U'k':
    break;
  case U'l':
    if (match<token::type::kw_lazy>())
      return consume<token::type::kw_lazy>();
    if (match<token::type::kw_left>())
      return consume<token::type::kw_left>();
    if (match<token::type::kw_let>())
      return consume<token::type::kw_let>();
    break;
  case U'm':
    if (match<token::type::kw_mutating>())
      return consume<token::type::kw_mutating>();
    break;
  case U'n':
    if (match<token::type::kw_none>())
      return consume<token::type::kw_none>();
    if (match<token::type::kw_nonmutating>())
      return consume<token::type::kw_nonmutating>();
    break;
  case U'o':
    if (match<token::type::kw_operator>())
      return consume<token::type::kw_operator>();
    if (match<token::type::kw_optional>())
      return consume<token::type::kw_optional>();
    if (match<token::type::kw_override>())
      return consume<token::type::kw_override>();
    break;
  case U'p':
    if (match<token::type::kw_postfix>())
      return consume<token::type::kw_postfix>();
    if (match<token::type::kw_precedence>())
      return consume<token::type::kw_precedence>();
    if (match<token::type::kw_prefix>())
      return consume<token::type::kw_prefix>();
    if (match<token::type::kw_private>())
      return consume<token::type::kw_private>();
    if (match<token::type::kw_protocol>())
      return consume<token::type::kw_protocol>();
    if (match<token::type::kw_public>())
      return consume<token::type::kw_public>();
    break;
  case U'q':
    break;
  case U'r':
    if (match<token::type::kw_repeat>())
      return consume<token::type::kw_repeat>();
    if (match<token::type::kw_rethrows>())
      return consume<token::type::kw_rethrows>();
    if (match<token::type::kw_return>())
      return consume<token::type::kw_return>();
    if (match<token::type::kw_required>())
      return consume<token::type::kw_required>();
    if (match<token::type::kw_right>())
      return consume<token::type::kw_right>();
    break;
  case U's':
    if (match<token::type::kw_self>())
      return consume<token::type::kw_self>();
    if (match<token::type::kw_set>())
      return consume<token::type::kw_set>();
    if (match<token::type::kw_static>())
      return consume<token::type::kw_static>();
    if (match<token::type::kw_struct>())
      return consume<token::type::kw_struct>();
    if (match<token::type::kw_subscript>())
      return consume<token::type::kw_subscript>();
    if (match<token::type::kw_super>())
      return consume<token::type::kw_super>();
    if (match<token::type::kw_switch>())
      return consume<token::type::kw_switch>();
    break;
  case U't':
    if (match<token::type::kw_try>())
      return consume<token::type::kw_try>();
    if (match<token::type::kw_typealias>())
      return consume<token::type::kw_typealias>();
    break;
  case U'u':
    if (match<token::type::kw_unowned>())
      return consume<token::type::kw_unowned>();
    break;
  case U'v':
    if (match<token::type::kw_var>())
      return consume<token::type::kw_var>();
    break;
  case U'w':
    if (match<token::type::kw_weak>())
      return consume<token::type::kw_weak>();
    if (match<token::type::kw_where>())
      return consume<token::type::kw_where>();
    if (match<token::type::kw_while>())
      return consume<token::type::kw_while>();
    if (match<token::type::kw_willSet>())
      return consume<token::type::kw_willSet>();
    break;
  case U'x':
  case U'y':
  case U'z':
    break;
  case U'P':
    if (match<token::type::kw_Protocol>())
      return consume<token::type::kw_Protocol>();
    break;
  case U'S':
    if (match<token::type::kw_Self>())
      return consume<token::type::kw_Self>();
  case U'T':
    if (match<token::type::kw_Type>())
      return consume<token::type::kw_Type>();
    break;
  }

  if (match<token::type::literal>())
    return consume<token::type::literal>();
  return consume<token::type::identifier>();
}

token lexer::head() {
  if (lookahead_.empty())
    lookahead_.push_back(next());
  return lookahead_.front();
}

token lexer::peek() {
  while (lookahead_.size() < 2) {
    lookahead_.push_back(lex());
    if (lookahead_.back().is<token::type::eof>())
      break;
  }
  return lookahead_.back();
}

token lexer::next() {
  if (!lookahead_.empty()) {
    auto token = lookahead_.front();
    lookahead_.pop_front();
    return token;
  }

  return lex();
}

void lexer::set_buffer(const char32_t *buffer, size_t length) {
  // XXX(compnerd) should we assert that the current buffer has been exhaused or
  // is invalid (nullptr, 0) when a new buffer is provided?
  buffer_start_ = buffer;
  buffer_end_ = buffer + length;
  cursor_ = buffer_start_;
  line_ = 1;
  column_ = 0;
  lookahead_.clear();
}
}

