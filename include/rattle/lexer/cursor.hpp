#pragma once

#include "api.hpp"
#include "error.hpp"
#include <cassert>
#include <rattle/token/token.hpp>
#include <string_view>
#include <type_traits>

namespace rattle::lexer::internal {
  // Represents a snapshot of where we are.
  struct State {
    token::Location location;
    std::string_view::const_iterator iterator;
  };
  // Cursor base, provides common operations while abstracting
  // low level iterator gymnastics
  class Cursor {
    // The program being lexed
    std::string_view program;
    // Marks the start of lexeme buffer and end, aka current.
    // If `start == current`, then buffer is empty.
    State start, current;
    // Where the current line being lexed started
    std::string_view::const_iterator line_start;
    // Event reactor (notified on preselected events)
    IReactor &reactor;

    // Flush lexeme buffer.
    // Since every character must belong to to one Token's lexeme
    // then flushing will be hidden only exposed as `make_token`.
    // Unless the lexer is drain in which the rest will not be owned by any token
    // NOTE: the current character is not yet consumed
    void flush_buffer() noexcept { start = current; }

  public:
    constexpr Cursor(std::string_view program, IReactor &reactor) noexcept
      : program{program}, start{token::Location::Valid(), program.cbegin()},
        current{token::Location::Valid(), program.cbegin()},
        line_start{program.cbegin()}, reactor{reactor} {}

    // Jump to the end of the program.
    void drain_program() noexcept {
      // NOTE: No more tokens after this call, only `Eot`.
      start.iterator = current.iterator = program.cend();
    }
    // Check if we are at the end of the program.
    bool empty() const noexcept { return current.iterator == program.cend(); }
    // Preconditions, nth char exists
    char peek(std::ptrdiff_t n = 0) const noexcept {
      assert(n < 0 ? safe_behind(-n) : safe(n));
      return *(current.iterator + n);
    }
    // Report an error to the manager
    void report(error::Error const &error) noexcept {
      if (reactor.report(error) == OnError::Abort) {
        drain_program();
      } /* else `OnError::Resume` */
    }
    void report(error::Kind error) noexcept {
      report({error, start.location, current.location, buffer()});
    }
    void report(error::Kind error, State mark) noexcept {
      report({error, mark.location, current.location,
        {mark.iterator, current.iterator}});
    }
    // Get location where lexeme started
    token::Location start_location() const noexcept { return start.location; }
    // Get current location; where we at?
    token::Location current_location() const noexcept {
      return current.location;
    }
    // Get current lexeme collected so far
    std::string_view buffer() const noexcept {
      return {start.iterator, current.iterator};
    }
    // Get current point in the program
    State bookmark() const noexcept { return current; }
    // Consume a character appropriately. [UNSAFE]
    // precondition: cursor has not reached end of file
    char eat() noexcept;
    // Check how far ahead we can peek safely
    std::size_t max_safe_ahead() const noexcept {
      return program.cend() - current.iterator;
    }
    // Check how far behind we can peek safely
    std::size_t max_safe_behind() const noexcept {
      return current.iterator - program.cbegin();
    }
    // Can we peek n characters ahead? Default `n=0` which checks if current character
    // is safe to peek
    bool safe(std::size_t n = 0) const noexcept { return n < max_safe_ahead(); }
    // Can we peek n characters behind? Default `n=0` which checks if current character
    // is safe to peek
    bool safe_behind(std::size_t n = 0) const noexcept {
      return n < max_safe_behind();
    }
    // Advance and return true if current character is what we expect
    // otherwise return false
    bool match(char expected) noexcept {
      return safe() and expected == peek() ? (eat(), true) : false;
    }
    // Advance and match if safe [UNSAFE: calls `eat`]
    bool match_next(char expected) noexcept { return (eat(), match(expected)); }
    // Make a token of kind
    token::Token make_token(token::Kind kind) noexcept {
      token::Token token{kind, start.location, current.location, buffer()};
      reactor.trace(token); // notify the reactor that a token was created
      flush_buffer(); // flush current lexeme
      return token;
    }
    // Report and make an error token
    token::Token make_token(error::Kind kind) noexcept {
      report(kind);
      return make_token(token::Kind::Error);
    }
    // Consume while some predicate is true
    template <class Predicate>
      requires std::is_convertible_v<std::invoke_result_t<Predicate, char>,
        bool>
    std::size_t eat_while(Predicate &&predicate) noexcept(
      std::is_nothrow_invocable_v<Predicate, char>) {
      std::size_t consumed = 0;
      while (safe() and predicate(peek())) {
        consumed++;
        eat();
      }
      return consumed;
    }
  };
} // namespace rattle::lexer::internal
