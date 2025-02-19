#pragma once

#include <cassert>
#include <rattle/token/token.hpp>

namespace rattle {
  // Interface of a lexer type
  struct ILexer {
    // Get the next token
    [[nodiscard]]
    virtual token::Token lex() noexcept = 0;
    // Once this returns true, the lexer is assumed
    // to always return `Eot`
    virtual bool empty() const noexcept = 0;
    // After this call, the lexer must always
    // stream `Eot` tokens; aka drain
    virtual void drain() noexcept {
      // Default implementation might not be the
      // best/efficient depending on the child class,
      // if there is a faster more efficient way,
      // they are free to override.
      while (not empty()) {
        (void)lex();
      }
    }
    virtual ~ILexer() = default;
  };
} // namespace rattle

