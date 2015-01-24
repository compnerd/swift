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

#include "interpreter.hh"
#include "line-editor.hh"
#include "stream.hh"

#include <codecvt>
#include <iomanip>
#include <iostream>
#include <string>

#include <swift/support/ucs4-support.hh>
#include <swift/syntax/printer.hh>

static const std::map<std::string, swift::interpreter::interpreter::command>
    commands{
      { "help", &swift::interpreter::interpreter::do_help },
      { "quit", &swift::interpreter::interpreter::do_quit },
    };

namespace swift {
namespace interpreter {
interpreter::interpreter()
    : count_(1), quit_(false), diagnostics_engine_(nullptr, this),
      lexer_(diagnostics_engine_, nullptr, 0),
      ast_context_(diagnostics_engine_), semantic_analyzer_(ast_context_),
      parser_(lexer_, semantic_analyzer_, diagnostics_engine_) {}

void interpreter::run_main_loop() {
  std::cout << "Welcome to Swift!  Type :help for assistance." << std::endl;

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs;
  line_editor editor("");
  editor.list_completer([&](std::u32string_view, size_t) {
    return std::vector<line_editor::completion>();
  });

  while (auto line = editor.readline()) {
    if ((*line)[0] == ':') {
      dispatch_command(line->substr(1));
      if (quit_)
        break;
      continue;
    }

    buffer_ = ucs.from_bytes(line->c_str());
    lexer_.set_buffer(buffer_.c_str(), buffer_.length());

    swift::parse::result<ast::statement> top_level_declaration =
        parser_.parse_top_level_declaration();
    if (not top_level_declaration)
      continue;

    ast::printer print(std::cerr, 2);
    print(top_level_declaration);
    std::cerr << '\n';

    ++count_;
  }
}

void interpreter::handle_diagnostic(diagnostics::diagnostic::level level,
                                    const diagnostics::diagnostic_info &info) {
  std::string message;
  info.format(message);

  std::cerr << swift::io::colour::white
            << "repl.swift:" << info.location().line() << ":"
            << info.location().column() << ": ";

  switch (level) {
  case swift::diagnostics::diagnostic::level::ignored:
    __builtin_trap();
  case swift::diagnostics::diagnostic::level::note:
    std::cerr << swift::io::colour::dark_grey << "note: ";
    break;
  case swift::diagnostics::diagnostic::level::warning:
    std::cerr << swift::io::colour::bright_magenta << "warning: ";
    break;
  case swift::diagnostics::diagnostic::level::error:
    std::cerr << swift::io::colour::bright_red << "error: ";
    break;
  case swift::diagnostics::diagnostic::level::fatal:
    __builtin_trap();
    break;
  }
  std::cerr << swift::io::colour::white << message << swift::io::colour::normal
            << '\n';

  // TODO(compnerd) properly index into buffer
  const std::u32string::size_type end =
      buffer_.find_first_of(U"\r\n", info.location().column());
  std::cerr << buffer_.substr(0, end) << std::endl;
  std::cerr << std::string(info.location().column(), ' ') << '^' << std::endl;
  std::cerr << std::endl;
}

void interpreter::dispatch_command(const std::string &string) {
  // FIXME(compnerd) support arguments
  auto entry = commands.find(string.c_str());
  if (entry == commands.end())
    std::cerr << "error: '" << string << "' is not a valid command."
              << std::endl;
  else
    (this->*entry->second)();
}

void interpreter::do_help() {
  std::cout << R"(
The Swift REPL (Read-Eval-Print-Loop) acts like an interpreter.  Valid statements, expressions, and declarations are immediately compiled and executed.

For more information on and command, type ':help <command-name>'.
)";
}

void interpreter::do_quit() {
  quit_ = true;
}
}
}

