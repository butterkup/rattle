#pragma once

#include <queue>
#include <stack>
#include "../lexer/scanner.hpp"
#include "../lexer/state.hpp"
#include "../lexer/tokens.hpp"
#include "../lexer/utility.hpp"

namespace rat::parser {
  class State {
    std::stack<lexer::Token> queued;
    lexer::State source;
  public:
    lexer::Token peek() {
      if ( not this->queued.size() )
        this->queued.push(lexer::scan(this->source));
      return this->queued.top();
    }

    inline bool empty() {
      return this->match(lexer::Token::Kind::Eot);
    }

    lexer::Token pop() {
      if ( this->queued.size() ) {
        lexer::Token tk = this->queued.top();
        return (this->queued.pop(), tk);
      }
      return lexer::scan(this->source);
    }

    inline lexer::Token push(lexer::Token tk) {
      this->queued.push(tk);
    }

    inline bool match(lexer::Token::Kind kind) {
      return this->peek().kind == kind;
    }

    inline void report_error(
      lexer::Location start,
      lexer::Location end,
      std::string_view msg,
      std::string_view fix = "") {
      this->source.report_error_at(
        start, end, msg, fix
      );
    }

    inline void report_error(
      lexer::Token token,
      std::string_view msg,
      std::string_view fix = "") {
      this->report_error(
        token.start, token.end, msg, fix
      );
    }

    inline void report_error(
      lexer::Token tk_start,
      lexer::Token tk_end,
      std::string_view msg,
      std::string_view fix = "") {
      this->report_error(
        tk_start.start, tk_end.end, msg, fix
      );
    }
  };
}
