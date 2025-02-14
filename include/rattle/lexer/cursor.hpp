#pragma once

#include "api.hpp"
#include "error.hpp"
#include <cassert>
#include <rattle/token/token.hpp>
#include <string_view>

namespace rattle::lexer::internal {
  // Represents a snapshot of where we are.
  struct State {
    token::Location location;
    std::string_view::const_iterator iterator;
  };
  // Cursor base, provides common operations while abstracting
  // low level iterator gymnastics
  class Cursor {
    const std::string_view program;
    // Marks the start of lexeme buffer and end, aka current.
    // If `start == current`, then buffer is empty.
    State start, current;
    // Where the current line being lexed started
    std::string_view::const_iterator line_start;
    IReactor &reactor;

  public:
    Cursor(const std::string_view program, IReactor &reactor)
      : program{program}, start{token::Location::Valid(), program.cbegin()},
        current{token::Location::Valid(), program.cbegin()},
        line_start{program.cbegin()}, reactor{reactor} {}

    // Flush lexeme buffer
    // NOTE: the current character is not yet consumed
    void flush_lexeme() { start = current; }
    // Jump to the end of the program.
    void drain_program() { start.iterator = current.iterator = program.cend(); }
    // Check if we are at the end of the program.
    bool empty() const { return current.iterator == program.cend(); }
    // Preconditions, nth char exists
    char peek(std::ptrdiff_t n = 0) const {
      assert(safe(n));
      return *(current.iterator + n);
    }
    // Report an error to the manager
    void report(error::Error error) {
      if (reactor.report(error) == OnError::Abort) {
        drain_program();
      } /* else `OnError::Resume` */
    }
    void report(error::Kind error) {
      report({error, start.location, current.location, buffer()});
    }
    void report(error::Kind error, State mark) {
      report({error, mark.location, current.location,
        {mark.iterator, current.iterator}});
    }
    // Get location where lexeme started
    token::Location start_location() const { return start.location; }
    // Get current location; where we at?
    token::Location current_location() const { return current.location; }
    // Get current lexeme collected so far
    std::string_view buffer() const {
      return {start.iterator, current.iterator};
    }
    // Get current point in the program
    State bookmark() const { return current; }
    // Consume a character appropriately. [UNSAFE]
    // precondition: cursor has not reached end of file
    char eat();
    // Check how far ahead we can peek safely
    std::ptrdiff_t max_safe() const {
      return program.cend() - current.iterator;
    }
    // Can we peek n characters ahead, default 0 which checks if current character
    // is safe to peek
    bool safe(std::size_t n = 0) const { return n < max_safe(); }
    // Advance and return true if current character is what we expect
    // otherwise return false
    bool match(char expected) {
      return safe() and expected == peek() ? (eat(), true) : false;
    }
    // Advance and match if safe [UNSAFE: calls `eat`]
    bool match_next(char expected) { return (eat(), match(expected)); }
    // Make a token of kind
    token::Token make_token(token::Kind kind) {
      token::Token token{kind, start.location, current.location, buffer()};
      reactor.trace(token);
      flush_lexeme();
      return token;
    }
    // Report and make an error token
    token::Token make_token(error::Kind kind) {
      report(kind);
      return make_token(token::Kind::Error);
    }
    // Consume while some predicate is true
    template <class predicate_t>
    std::size_t eat_while(predicate_t &&predicate) {
      std::size_t consumed = 0;
      while (not empty() and predicate(peek())) {
        consumed++;
        eat();
      }
      return consumed;
    }
  };
} // namespace rattle::lexer::internal
