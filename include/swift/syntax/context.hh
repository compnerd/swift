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

#ifndef swift_syntax_context_hh
#define swift_syntax_context_hh

#include <llvm/Support/Allocator.h>

namespace swift {
namespace ast {
class context;
}
namespace compiler {
class target_info;
}
namespace diagnostics {
class engine;
}
}

inline void *operator new(size_t, const swift::ast::context &, size_t);
inline void *operator new[](size_t, const swift::ast::context &, size_t);

inline void operator delete(void *, const swift::ast::context &, size_t);
inline void operator delete[](void *, const swift::ast::context &, size_t);

namespace swift {
namespace ast {
class source_file;

class context {
  friend void * ::operator new(size_t, const context &, size_t);
  friend void * ::operator new[](size_t, const context &, size_t);
  friend void ::operator delete(void *, const swift::ast::context &, size_t);
  friend void ::operator delete[](void *, const swift::ast::context &, size_t);

  mutable llvm::BumpPtrAllocator allocator_;

  diagnostics::engine &diagnostics_engine_;
  const compiler::target_info *target_info_;

  ast::source_file *source_file_;

public:
  context(diagnostics::engine &engine);

  ast::source_file *source_file() const {
    return source_file_;
  }

  diagnostics::engine &diagnostics_engine() const {
    return diagnostics_engine_;
  }

  void initialise_builtin_types(const compiler::target_info &target);
};
}
}

inline void *operator new(size_t size, const swift::ast::context &context,
                          size_t alignment = 8) {
  return context.allocator_.Allocate(size, alignment);
}

inline void *operator new[](size_t size, const swift::ast::context &context,
                            size_t alignment = 8) {
  return context.allocator_.Allocate(size, alignment);
}

inline void operator delete(void *, const swift::ast::context &, size_t) {
  /* unsupported */
}

inline void operator delete[](void *, const swift::ast::context &, size_t) {
  /* unsupported */
}

#endif

