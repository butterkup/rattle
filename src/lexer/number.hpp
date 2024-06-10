#pragma once

#include "state.hpp"
#include "tokens.hpp"

namespace rat::lexer {
  namespace number_lexer { } // facade of how numbers are lexed

  Token::Kind consume_number(State& S) { }
}
