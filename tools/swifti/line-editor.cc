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

#include "line-editor.hh"
#if defined(HAVE_LIBEDIT)
#include <ext/string_view>
#include <codecvt>
#include <sstream>

#include <histedit.h>
#endif

static std::string
common_prefix(const std::vector<swift::interpreter::line_editor::completion>
                  &completions) {
  assert(!completions.empty() && "must have at least one completion");

  std::string prefix = completions[0].input_text();
  for (const auto &completion : completions) {
    const auto &input = completion.input_text();
    size_t length = std::min(prefix.size(), input.size());
    size_t match;
    for (match = 0; match < length; ++match)
      if (prefix[match] != input[match])
        break;
    prefix.resize(match);
  }
  return prefix;
}

namespace swift::interpreter {
line_editor::completion_action
line_editor::get_completion_action(std::u32string_view buffer,
                                   size_t position) const {
  if (completer_)
    return completer_->complete(buffer, position);
  return completion_action(line_editor::completion_action::type::completions);
}

line_editor::completion_action
line_editor::list_completer_concept::complete(std::u32string_view buffer,
                                              size_t position) const {
  std::vector<completion> completions = get_completions(buffer, position);
  if (completions.empty())
    return completion_action(completion_action::type::completions);

  std::string prefix = common_prefix(completions);
  if (prefix.empty())
    return completion_action(prefix);

  std::vector<std::string> suggestions;
  std::for_each(std::begin(completions), std::end(completions),
                [&suggestions](const auto &completion) {
                  suggestions.push_back(completion.display_text());
                });
  return completion_action(suggestions);
}

#if defined(HAVE_LIBEDIT)
struct line_editor::state {
  line_editor *line_editor;
  EditLine *el;
  FILE *out;

  unsigned index;
  std::string completion;
};

static const char *ElGetPromptFn(EditLine *el) {
  line_editor::state *state;
  if (el_get(el, EL_CLIENTDATA, &state) == 0)
    return state->line_editor->prompt().c_str();
  return "> ";
}

static unsigned char ElCompletionFn(EditLine *el, int) {
  line_editor::state *state;
  if (el_get(el, EL_CLIENTDATA, &state))
    return CC_ERROR;

  if (!state->completion.empty()) {
    ::fwrite(state->completion.c_str(), state->completion.size(), 1, state->out);

    std::string previous(state->index, '\02');
    ::el_push(el, const_cast<char *>(previous.c_str()));

    state->completion.clear();
    return CC_REFRESH;
  }

  const LineInfo *line = ::el_line(el);
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> ucs;
  std::u32string_view input(ucs.from_bytes(line->buffer).c_str(),
                            line->lastchar - line->buffer);
  line_editor::completion_action action =
      state->line_editor->get_completion_action(input,
                                                line->cursor - line->buffer);
  switch (action.type()) {
  case line_editor::completion_action::type::insert:
    ::el_insertstr(el, action.insert().c_str());
    return CC_REFRESH;

  case line_editor::completion_action::type::completions:
    if (action.completions().empty())
      return CC_REFRESH_BEEP;

    ::el_push(el, const_cast<char *>("\05\t"));

    std::ostringstream oss;
    oss << '\n';
    for (const auto &completion : action.completions())
      oss << completion << '\n';
    oss << state->line_editor->prompt()
        << std::string_view(line->buffer, line->lastchar - line->buffer);
    state->completion = oss.str();
    state->index = line->lastchar - line->cursor;
    return CC_REFRESH;
  }
}

line_editor::line_editor(const std::string &program, FILE *in, FILE *out,
                         FILE *err)
    : prompt_(program + "> "), state_(new state) {
  state_->line_editor = this;
  state_->el = ::el_init(program.c_str(), in, out, err);
  assert(state_->el && "el_init failed");
  state_->out = out;

  ::el_set(state_->el, EL_PROMPT, ElGetPromptFn);
  ::el_set(state_->el, EL_EDITOR, "emacs");
  ::el_set(state_->el, EL_ADDFN, "tab_complete", "", ElCompletionFn);
  ::el_set(state_->el, EL_BIND, "\t", "tab_complete", NULL);
  ::el_set(state_->el, EL_BIND, "^r", "em-inc-search-prev", NULL);
  ::el_set(state_->el, EL_BIND, "^w", "em-delete-prev-word", NULL);
  ::el_set(state_->el, EL_BIND, "\033[3~", "ed-delete-next-char", NULL);
  ::el_set(state_->el, EL_CLIENTDATA, state_.get());
}

line_editor::~line_editor() {
  ::el_end(state_->el);
  ::fwrite("\n", 1, 1, state_->out);
}

std::optional<std::string> line_editor::readline() const {
  int length = 0;
  const char *line = ::el_gets(state_->el, &length);

  if (not line or not length)
    return std::optional<std::string>();

  while (length and (line[length - 1] == '\n' or line[length - 1] == '\r'))
    --length;

  return std::string(line, length);
}
#else
struct line_editor::state {
  FILE *in;
  FILE *out;
  FILE *err;
};

line_editor::line_editor(const std::string &program, FILE *in, FILE *out,
                         FILE *err)
    : prompt_(program + "> "), state_(new state) {
  state_->in = in;
  state_->out = out;
  state_->err = err;
}

line_editor::~line_editor() {
  ::fwrite("\n", 1, 1, state_->out);
}

std::optional<std::string> line_editor::readline() const {
  ::fprintf(state_->out, "%s", prompt_.c_str());

  std::string line;
  do {
    char buffer[64];
    if (::fgets(buffer, sizeof(buffer), state_->in))
      line.append(buffer);
    else
      return line.empty() ? std::optional<std::string>() : line;
  } while (line.empty() or
           (line[line.size() - 1] != '\n' and line[line.size() - 1] != '\r'));

  while (not line.empty() and
         (line[line.size() - 1] == '\n' or line[line.size() - 1] == '\r'))
    line.resize(line.size() - 1);

  return line;
}
#endif
}
