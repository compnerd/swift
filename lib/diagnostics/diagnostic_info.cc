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

#include "swift/diagnostics/diagnostic_info.hh"
#include "swift/diagnostics/diagnostics.hh"
#include "swift/diagnostics/engine.hh"
#include "swift/diagnostics/ids.hh"
#include "swift/support/ucs4-support.hh"

#include <sstream>

using diagnostic = swift::diagnostics::diagnostic;
using level = swift::diagnostics::ids::level;

namespace swift {
namespace diagnostics {
void diagnostic_info::format(std::string &message) const {
  std::ostringstream oss;
  const char *string = diagnostic::get_format(id());
  for (const char *p = string; p && *p; ++p) {
    if (*p == '%') {
      switch (*++p) {
      default:
        oss << '%' << *p;
      case '%':  // %%
        oss << '%';
        break;
      case '0' ... '9': {
        int argument_index = *p - '0';
        assert(argument_index < engine_->arguments_ &&
               "insufficient arguments to format");
        switch (engine_->argument_types_[argument_index]) {
        case engine::argument_type::string:
          oss << engine_->argument_strings_[argument_index];
          break;
        case engine::argument_type::u32string:
          oss << engine_->argument_u32strings_[argument_index];
          break;
        case engine::argument_type::signed_integer:
          oss << static_cast<intmax_t>(engine_->argument_values_[argument_index]);
          break;
        case engine::argument_type::unsigned_integer:
          oss << static_cast<uintmax_t>(engine_->argument_values_[argument_index]);
          break;
        }
        break;
      }
      }
    } else {
      oss << *p;
    }
  }
  message = oss.str();
}
}
}
