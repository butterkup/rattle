#pragma once

#include "api.hpp"
#include "cursor.hpp"
#include <rattle/lexer.hpp>
#include <rattle/token/token.hpp>
#include <rattle/utility.hpp>
#include <string_view>

namespace rattle::lexer {
  namespace internal {
    // Actual lexer, moves through the program creating tokens.
    // Acts highly of the state, managers the cursor.
    struct Lexer {
      Lexer(std::string_view program, IReactor &reactor)
        : base{program, reactor} {}
      // Scan the next token
      token::Token scan();
      bool empty() const { return base.empty(); }
      // Jump to the end of program
      void drain() { base.drain_program(); }

    private:
      // manages position in the program (aka actual/core cursor)
      Cursor base;
      // They do as they say and wrap as a token
      token::Token consume_whitespace();
      token::Token consume_comment();
      token::Token consume_string();
      token::Token consume_number();
      token::Token consume_identifier();
    };
  } // namespace internal

  // A facade; Bridge between the actual implementation
  // and to conforn to `ILexer`
  struct Lexer: ILexer {
    Lexer(std::string_view program, IReactor &reactor)
      : lexer{program, reactor} {}

    token::Token lex() override { return lexer.scan(); }
    bool empty() const override { return lexer.empty(); }
    void drain() override { lexer.drain(); }

  private:
    internal::Lexer lexer;
  };
  // For cohesiveness
  using utility::OnError;
} // namespace rattle::lexer
