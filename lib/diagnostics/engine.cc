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

#include "swift/diagnostics/engine.hh"
#include "swift/diagnostics/consumer.hh"

namespace swift {
namespace diagnostics {
engine::engine(diagnostics::options *options, diagnostics::consumer *consumer)
    : options_(options), consumer_(consumer),
      current_diagnostic_id_(diagnostic::invalid) {}

engine::~engine() {}

void engine::emit_current_diagnostic() {
  assert(consumer_ && "diagnostic consumer not set");
  process_diagnostic();
  clear_diagnostic();
}

void engine::process_diagnostic() {
  assert(not (current_diagnostic_id_ == diagnostic::invalid) &&
         "no current diagnostic");
  diagnostics::diagnostic_info info(this);
  // FIXME(compnerd) hoist out the ids to actually be able to share that
  consumer_->handle_diagnostic(diagnostic::get_level(info.id()), info);
}
}
}

