#pragma once

#include "api.hpp"
#include "error.hpp"
#include <cassert>
#include <rattle/lexer.hpp>
#include <rattle/token/token.hpp>

namespace rattle::parser::internal {
  struct State {
    enum Filter : int {
      // Skip nothing, not even whitespace.
      NONE = 0,
      // Skip ERROR token
      ERROR = 1,
      // Skip ESCAPE token
      ESCAPE = 1 << 1,
      // Skip NEWLINE token
      NEWLINE = 1 << 2,
      // Skip COMMENT token
      COMMENT = 1 << 3,
      // Skip WHITESPACE token
      WHITESPACE = 1 << 4,
      // Cannot imagine why, but here we are.
      ALL = 0xFF,

      // Skip COM, WS, ESC and ERR tokens
      EOS = COMMENT | WHITESPACE | ESCAPE | ERROR,

      // Skip WS, ERR and ESC tokens
      DEFAULT = WHITESPACE | ERROR | ESCAPE
    };

    bool skip(token::Token const &token) const {
      switch (token.kind) {
      // Handling `token::Kind::Eot` might lead to infinite loop
      case token::Kind::Error:
        return flags & State::Filter::ERROR;
      case token::Kind::Newline:
        return flags & State::Filter::NEWLINE;
      case token::Kind::Whitespace:
        return flags & State::Filter::WHITESPACE;
      case token::Kind::Pound:
        return flags & State::Filter::COMMENT;
      case token::Kind::Escape:
        return flags & State::Filter::ESCAPE;
      default:
        return false;
      }
    }

    token::Token first;
    Filter flags;
  };

  class Flags;

  class Cursor {
    State state;
    ILexer &lexer;
    IReactor &reactor;

  public:
    Cursor(ILexer &lexer, IReactor &reactor)
      : state{lexer.lex(), State::Filter::DEFAULT}, lexer{lexer},
        reactor{reactor} {}

    State::Filter flags() const { return state.flags; }
    State::Filter flags(State::Filter flags) {
      auto oflags = state.flags;
      state.flags = flags;
      return oflags;
    }
    State::Filter flags(State::Filter flags, unsigned mask) {
      auto oflags = state.flags;
      state.flags = static_cast<State::Filter>((oflags & mask) | flags);
      return oflags;
    }

    Flags with();
    Flags with(State::Filter flags);
    Flags with(State::Filter flags, unsigned mask);

    void drain_program() {
      // Drain the lexer
      lexer.drain();
      // Lexer must now be empty, from here on, it streams `token::Kind::Eot`
      assert(lexer.empty());
      // Set current to token of kind `token::Kind::Eot`
      state.first = lexer.lex();
      // Assert for testing purposes
      assert(iskind(token::Kind::Eot));
    }

    bool empty() const { return iskind(token::Kind::Eot); }
    token::Token const &peek() const { return state.first; }
    bool iskind(token::Kind kind) const { return iskind() == kind; }
    token::Kind iskind() const { return state.first.kind; }

    void report(error::Error error) {
      if (reactor.report(error) == OnError::Abort) {
        drain_program();
      } /* else `OnError::Resume` */
    }
    void report(error::Kind kind) { report({kind, state.first}); }
    void report(error::Kind kind, token::Token where) { report({kind, where}); }

    // Traverse the token stream discarding unwanted tokens as per
    // `State::Filter` flags, once a wanted one is found, it is set
    // and returns the replaced token.
    token::Token eat() {
      token::Token eaten = state.first;
      for (;;) {
        token::Token token = lexer.lex();
        if (not state.skip(token)) {
          state.first = token;
          break;
        }
      }
      return eaten;
    }

    bool match(token::Kind kind) {
      return iskind(kind) ? (eat(), true) : false;
    }
    bool match_next(token::Kind kind) { return (eat(), match(kind)); }
  };

  // Delegate flags resetting to RAII.
  class Flags {
    Cursor &cursor;
    State::Filter flags;

  public:
    Flags(Cursor &cursor): cursor{cursor}, flags{cursor.flags()} {}
    Flags(Cursor &cursor, State::Filter nflags): cursor{cursor} {
      flags = cursor.flags(nflags);
    }
    ~Flags() {
      // Reset to the original saved flags state
      cursor.flags(flags);
    }
  };

  inline Flags Cursor::with() { return {*this}; }
  inline Flags Cursor::with(State::Filter nflags) {
    return {*this, flags(nflags)};
  }
  inline Flags Cursor::with(State::Filter nflags, unsigned mask) {
    return {*this, flags(nflags, mask)};
  }
} // namespace rattle::parser::internal

