#include "parser.hpp"
#include "precedence.hpp"

namespace rattle::parser {
  lexer::Token get(State &state, bool ignore_eos, bool ignore_comments) {
    while (true) {
      lexer::Token token = state.get();
      if (ignore_eos and token.kind == lexer::Token::Kind::Eos) {
        continue;
      }
      return token;
    }
  }
  prec operator--(prec const &p) { return prec(static_cast<unsigned>(p) - 1); }
  prec operator++(prec const &p) { return prec(static_cast<unsigned>(p) + 1); }
} // namespace rattle::parser

