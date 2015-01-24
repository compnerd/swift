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

#ifndef swift_diagnostics_consumer_hh
#define swift_diagnostics_consumer_hh

#include "swift/diagnostics/diagnostic_info.hh"
#include "swift/diagnostics/diagnostics.hh"
#include "swift/diagnostics/engine.hh"

namespace swift {
namespace diagnostics {
class diagnostic_info;

class consumer {
protected:
  unsigned warning_count_;
  unsigned error_count_;

public:
  consumer() : warning_count_(0), error_count_(0) {}
  virtual ~consumer();

  unsigned warning_count() const {
    return warning_count_;
  }
  unsigned error_count() const {
    return error_count_;
  }

  virtual void reset() {
    warning_count_ = 0;
    error_count_ = 0;
  }

  virtual void begin_source_file() {}
  virtual void end_source_file() {}

  virtual void
  handle_diagnostic(diagnostic::level level, const diagnostic_info &diag);
};

class null_consumer : public consumer {
public:
  virtual void handle_diagnostic(diagnostic::level, const diagnostic_info &) {}
};

class proxy_consumer : public consumer {
  consumer &target_;

public:
  proxy_consumer(consumer &target) : target_(target) {}
  virtual ~proxy_consumer() = default;

  void reset() override;

  void handle_diagnostic(diagnostic::level level,
                         const diagnostic_info &info) override;
};
}
}

#endif

