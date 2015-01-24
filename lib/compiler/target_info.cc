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

#include "swift/compiler/target_info.hh"
#include "swift/compiler/target_options.hh"
#include "swift/diagnostics/engine.hh"

#include <llvm/ADT/Triple.h>

namespace target {
class x86 : public swift::compiler::target_info {
public:
  x86(const llvm::Triple &triple) {
    (void)triple;
    endian_ = endian::little;
  }
};

class x86_64 : public x86 {
public:
  x86_64(const llvm::Triple &triple) : x86(triple) {}
};

template <typename architecture_>
class os_info : public architecture_ {
public:
  os_info(const llvm::Triple &triple) : architecture_(triple) {}
};

template <typename architecture_>
class darwin : public os_info<architecture_> {
public:
  darwin(const llvm::Triple &triple) : os_info<architecture_>(triple) {}
};

template <typename architecture_>
class windows : public os_info<architecture_> {
public:
  windows(const llvm::Triple &triple) : os_info<architecture_>(triple) {}
};

template <typename architecture_>
class linux : public os_info<architecture_> {
public:
  linux(const llvm::Triple &triple) : os_info<architecture_>(triple) {}
};
}

static swift::compiler::target_info *
allocate_target(const llvm::Triple &triple) {
  switch (triple.getArch()) {
  default:
    return nullptr;
  case llvm::Triple::arm:
    __builtin_trap();
    break;
  case llvm::Triple::x86:
    __builtin_trap();
    break;
  case llvm::Triple::x86_64:
    switch (triple.getOS()) {
    default:
      return nullptr;
    case llvm::Triple::Linux:
      return new target::linux<target::x86_64>(triple);
    case llvm::Triple::Win32:
      __builtin_trap();
    case llvm::Triple::Darwin:
      return new target::darwin<target::x86_64>(triple);
    }
    break;
  }
}

namespace swift {
namespace compiler {
using namespace swift::diagnostics;

target_info *target_info::create(diagnostics::engine &diagnostics_engine,
                                 const target_options *target_options) {
  if (auto target = allocate_target(llvm::Triple(target_options->triple)))
    return target;

  diagnostics_engine.report(diagnostic::err_target_unknown_triple)
      << target_options->triple;
  return nullptr;
}
}
}
