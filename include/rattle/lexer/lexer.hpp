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
      token::Token scan() noexcept;
      bool empty() const noexcept { return base.empty(); }
      // Jump to the end of program
      void drain() noexcept { base.drain_program(); }

    private:
      // manages position in the program (aka actual/core cursor)
      Cursor base;
      // They do as they say and wrap as a token
      token::Token consume_whitespace() noexcept;
      token::Token consume_comment() noexcept;
      token::Token consume_string() noexcept;
      token::Token consume_raw_string() noexcept;
      token::Token consume_number() noexcept;
      token::Token consume_identifier() noexcept;
    };
  } // namespace internal

  // A facade; Bridge between the actual implementation
  // and to conforn to `ILexer`
  struct Lexer: ILexer {
    Lexer(std::string_view program, IReactor &reactor)
      : lexer{program, reactor} {}

    token::Token lex() noexcept override { return lexer.scan(); }
    bool empty() const noexcept override { return lexer.empty(); }
    void drain() noexcept override { lexer.drain(); }

  private:
    internal::Lexer lexer;
  };
} // namespace rattle::lexer
