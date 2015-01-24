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

#ifndef swift_diagnostics_engine_hh
#define swift_diagnostics_engine_hh

#include "swift/diagnostics/diagnostics.hh"
#include "swift/lexer/location.hh"
#include "swift/lexer/token.hh"

#include <ext/string_view>

namespace swift {
namespace diagnostics {
class builder;
class consumer;
class diagnostic;
class options;

class engine {
  friend class builder;
  friend class diagnostic_info;

  engine(const engine &) = delete;
  engine &operator=(const engine &) = delete;

  options *options_;
  consumer *consumer_;

  enum class argument_type {
    string,
    u32string,
    signed_integer,
    unsigned_integer,
  };

  static constexpr const size_t maximum_arguments = 10;

  range range_;
  diagnostic::id current_diagnostic_id_;
  signed char arguments_;
  argument_type argument_types_[maximum_arguments];
  std::string_view argument_strings_[maximum_arguments];
  std::u32string_view argument_u32strings_[maximum_arguments];
  intptr_t argument_values_[maximum_arguments];

protected:
  void emit_current_diagnostic();
  void process_diagnostic();
  void clear_diagnostic() {
    current_diagnostic_id_ = diagnostic::invalid;
    arguments_ = 0;
  }

public:
  enum class argument_kind {
    string,
    signed_integer,
    unsigned_integer,
    token,
  };

  explicit engine(diagnostics::options *options,
                  diagnostics::consumer *consumer = nullptr);
  ~engine();

  const diagnostics::options &options() const {
    return *options_;
  }

  diagnostics::consumer *consumer() {
    return consumer_;
  }
  const diagnostics::consumer *consumer() const {
    return consumer_;
  }
  void consumer(diagnostics::consumer *consumer);

  inline builder report(swift::range range, diagnostic::id id);
  inline builder report(diagnostic::id id);
};

class builder {
  friend class engine;

  mutable engine *engine_;
  mutable bool active_;

  builder &operator=(const builder &) = delete;

  builder() : engine_(nullptr), active_(false) {}
  explicit builder(engine *engine) : engine_(engine), active_(true) {}

protected:
  void reset() const {
    engine_ = nullptr;
    active_ = false;
  }

  void emit() {
    if (not active_)
      return;

    engine_->emit_current_diagnostic();
    reset();
  }

public:
  builder(const builder &rhs) : engine_(rhs.engine_), active_(rhs.active_) {
    rhs.reset();
  }

  ~builder() {
    emit();
  }

  void add(std::string_view string) const {
    assert(active_ && "diagnostic is inactive");
    assert(static_cast<size_t>(engine_->arguments_) <
               diagnostics::engine::maximum_arguments &&
           "too many arguments for diagnostic");

    engine_->argument_types_[engine_->arguments_] =
        diagnostics::engine::argument_type::string;
    engine_->argument_strings_[engine_->arguments_] = string;
    ++engine_->arguments_;
  }

  void add(std::u32string_view string) const {
    assert(active_ && "diagnostic is inactive");
    assert(static_cast<size_t>(engine_->arguments_) <
               diagnostics::engine::maximum_arguments &&
           "too many arguments for diagnostic");

    engine_->argument_types_[engine_->arguments_] =
        diagnostics::engine::argument_type::u32string;
    engine_->argument_u32strings_[engine_->arguments_] = string;
    ++engine_->arguments_;
  }

  void add(int integer) const {
    assert(active_ && "diagnostic is inactive");
    assert(static_cast<size_t>(engine_->arguments_) <
               diagnostics::engine::maximum_arguments &&
           "too many arguments for diagnostic");

    engine_->argument_types_[engine_->arguments_] =
        diagnostics::engine::argument_type::signed_integer;
    engine_->argument_values_[engine_->arguments_] = integer;
    ++engine_->arguments_;
  }

  void add(unsigned integer) const {
    assert(active_ && "diagnostic is inactive");
    assert(static_cast<size_t>(engine_->arguments_) <
               diagnostics::engine::maximum_arguments &&
           "too many arguments for diagnostic");

    engine_->argument_types_[engine_->arguments_] =
        diagnostics::engine::argument_type::unsigned_integer;
    engine_->argument_values_[engine_->arguments_] = integer;
    ++engine_->arguments_;
  }
};

inline builder engine::report(swift::range range, diagnostic::id id) {
  assert(current_diagnostic_id_ == diagnostic::invalid &&
         "only a single diagnostic may be emitted at once");
  range_ = range;
  current_diagnostic_id_ = id;
  return builder(this);
}

inline builder engine::report(diagnostic::id id) {
  return report(location(), id);
}
}
}

inline const swift::diagnostics::builder &
operator<<(const swift::diagnostics::builder &builder,
           std::string_view string) {
  builder.add(string);
  return builder;
}

inline const swift::diagnostics::builder &
operator<<(const swift::diagnostics::builder &builder,
           std::u32string_view string) {
  builder.add(string);
  return builder;
}

inline const swift::diagnostics::builder &
operator<<(const swift::diagnostics::builder &builder, int integer) {
  builder.add(integer);
  return builder;
}

inline const swift::diagnostics::builder &
operator<<(const swift::diagnostics::builder &builder, unsigned integer) {
  builder.add(integer);
  return builder;
}

inline const swift::diagnostics::builder &
operator<<(const swift::diagnostics::builder &builder,
           const swift::token &token) {
  builder.add(token.value());
  return builder;
}

#endif

