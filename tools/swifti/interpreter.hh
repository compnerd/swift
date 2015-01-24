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

#ifndef swifti_interpreter_hh
#define swifti_interpreter_hh

#include <map>
#include <string>

#include <swift/diagnostics/consumer.hh>
#include <swift/diagnostics/diagnostics.hh>
#include <swift/diagnostics/engine.hh>
#include <swift/lexer/lexer.hh>
#include <swift/parser/parser.hh>
#include <swift/semantic/analyzer.hh>
#include <swift/syntax/context.hh>

namespace swift::interpreter {
class interpreter : private diagnostics::consumer {
  unsigned count_;
  bool quit_;

  swift::diagnostics::engine diagnostics_engine_;
  swift::lexer lexer_;
  swift::ast::context ast_context_;
  swift::semantic::analyzer semantic_analyzer_;
  swift::parser parser_;

  std::u32string buffer_;

public:
  using command = void (interpreter::*)();

  interpreter();

  void run_main_loop();
  void handle_diagnostic(diagnostics::diagnostic::level level,
                         const diagnostics::diagnostic_info &info) override;

public:  // FIXME(compnerd) make this private via friending the commands table
  void dispatch_command(const std::string &);
  void do_help();
  void do_quit();
};
}

#endif

