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

#ifndef swifti_line_editor_hh
#define swifti_line_editor_hh

#include <ext/optional>
#include <ext/string_view>
#include <string>
#include <vector>

namespace swift::interpreter {
class line_editor {
public:
  class completion_action {
  public:
    enum class type {
      insert,
      completions,
    };

  private:
    type type_;
    std::string insert_;
    std::vector<std::string> completions_;

  public:
    completion_action() = default;
    completion_action(completion_action::type type) : type_(type) {}
    completion_action(const std::string &completion) : type_(type::insert), insert_(completion) {}
    completion_action(const std::vector<std::string> & completions) : type_(type::completions), completions_(completions) {}

    completion_action::type type() const {
      return type_;
    }
    const std::string &insert() const {
      assert(type_ == type::insert && "non-insertion completion type");
      return insert_;
    }
    const std::vector<std::string> &completions() const {
      assert(type_ == type::completions && "non-completions completion type");
      return completions_;
    }
  };

  class completion {
    std::string text_, display_;

  public:
    completion() = default;
    completion(const std::string &input, const std::string &display)
        : text_(input), display_(display) {}

    const std::string &input_text() const {
      return text_;
    }
    const std::string &display_text() const {
      return display_;
    }
  };

  struct state;

private:
  struct completer_concept {
    ~completer_concept() = default;

    virtual completion_action
    complete(std::u32string_view buffer, size_t position) const = 0;
  };

  template <typename ValueType>
  class completer_model : completer_concept {
    ValueType value_;

  public:
    completer_model(ValueType value) : value_(value) {}

    completion_action
    complete(std::u32string_view buffer, size_t position) const override {
      return value_(buffer, position);
    }
  };

  struct list_completer_concept : completer_concept {
    ~list_completer_concept() = default;

    completion_action
    complete(std::u32string_view buffer, size_t position) const;

    virtual std::vector<completion>
    get_completions(std::u32string_view buffer, size_t position) const = 0;
  };

  template <typename ValueType>
  class list_completer_model : public list_completer_concept {
    ValueType value_;

  public:
    list_completer_model(ValueType value) : value_(value) {}

    std::vector<completion>
    get_completions(std::u32string_view buffer, size_t position) const override {
      return value_(buffer, position);
    }
  };

  std::string prompt_;
  std::unique_ptr<const completer_concept> completer_;

  std::unique_ptr<state> state_;

public:
  line_editor(const std::string &program, FILE *in = stdin, FILE *out = stdout,
              FILE *err = stderr);
  ~line_editor();

  std::optional<std::string> readline() const;

  template <typename Completer>
  void completer(Completer completer) {
    completer_.reset(new completer_model<Completer>(completer));
  }

  template <typename Completer>
  void list_completer(Completer completer) {
    completer_.reset(new list_completer_model<Completer>(completer));
  }

  completion_action
  get_completion_action(std::u32string_view buffer, size_t position) const;

  const std::string &prompt() const {
    return prompt_;
  }
  void prompt(const std::string &prompt) {
    prompt_ = prompt;
  }
};
}

#endif
