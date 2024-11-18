#pragma once

#include <rattle/lexer.hpp>

namespace rattle::lexer {
  void consume_space(State &lexer);
  Token consume_comment(State &lexer);
  Token consume_multi_string(State &lexer);
  Token consume_single_string(State &lexer);
  Token consume_number(State &lexer);
  Token consume_identifier(State &lexer);
  bool isalnum(char c);
  bool isspace(char c);
  bool numend(char c);
  bool isdec(char c);
  bool isoct(char c);
  bool ishex(char c);
  bool isbin(char c);
} // namespace rattle::lexer

