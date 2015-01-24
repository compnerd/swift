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

#include "stream.hh"

#include <stdio.h>
#include <unistd.h>

static const char * escape_code[] = {
  [static_cast<int>(swift::io::colour::normal)] = "\x1b[00m",
  [static_cast<int>(swift::io::colour::black)] = "\x1b[30m",
  [static_cast<int>(swift::io::colour::red)] = "\x1b[31m",
  [static_cast<int>(swift::io::colour::green)] = "\x1b[32m",
  [static_cast<int>(swift::io::colour::yellow)] = "\x1b[33m",
  [static_cast<int>(swift::io::colour::blue)] = "\x1b[34m",
  [static_cast<int>(swift::io::colour::magenta)] = "\x1b[35m",
  [static_cast<int>(swift::io::colour::cyan)] = "\x1b[36m",
  [static_cast<int>(swift::io::colour::grey)] = "\x1b[37m",
  [static_cast<int>(swift::io::colour::dark_grey)] = "\x1b[30;1m",
  [static_cast<int>(swift::io::colour::bright_red)] = "\x1b[31;1m",
  [static_cast<int>(swift::io::colour::bright_green)] = "\x1b[32;1m",
  [static_cast<int>(swift::io::colour::bright_yellow)] = "\x1b[33;1m",
  [static_cast<int>(swift::io::colour::bright_blue)] = "\x1b[34;1m",
  [static_cast<int>(swift::io::colour::bright_magenta)] = "\x1b[35;1m",
  [static_cast<int>(swift::io::colour::bright_cyan)] = "\x1b[36;1m",
  [static_cast<int>(swift::io::colour::white)] = "\x1b[37;1m",
};

namespace swift::io {
bool show_colours = ::isatty(STDERR_FILENO);
}

std::ostream & operator<<(std::ostream & os, swift::io::colour colour) {
  if (not swift::io::show_colours)
    return os;
  os << escape_code[static_cast<int>(colour)];
  return os;
}

