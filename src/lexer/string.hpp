#pragma once

#include "state.hpp"
#include "tokens.hpp"

namespace rat::lexer {
  namespace string_lexer { } // facade of how strings are lexed

  Token::Kind consume_string(State& S) { }
}
