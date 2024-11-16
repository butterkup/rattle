#pragma once

#include <rattle/lexer.hpp>

namespace rattle::lexer {
  void consume_space(State &lexer);
  token::Token consume_comment(State &lexer);
  token::Token consume_multi_string(State &lexer);
  token::Token consume_single_string(State &lexer);
  token::Token consume_number(State &lexer);
  token::Token consume_identifier(State &lexer);
  bool isalnum(char c);
  bool isdecimal(char c);
  bool isoctal(char c);
  bool ishexadecimal(char c);
  bool isbinary(char c);
} // namespace rattle::lexer

