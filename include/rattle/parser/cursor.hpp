#pragma once

#include "api.hpp"
#include "error.hpp"
#include <rattle/lexer.hpp>
#include <rattle/token/token.hpp>

namespace rattle::parser::internal {
  struct State {
    enum Filter : int {
      ERROR = 1,
      ESCAPE = 1 << 1,
      NEWLINE = 1 << 2,
      COMMENT = 1 << 3,
      WHITESPACE = 1 << 4,

      WSERES = WHITESPACE | ERROR | ESCAPE,
      COWSER = COMMENT | WSERES,

      INL = NEWLINE | COWSER,

      DEFAULT = WSERES
    };

    bool ignore(token::Token const &token) const {
      switch (token.kind) {
      // Handling `token::Eot` will lead to infinite loop
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

    token::Token current;
    Filter flags;
  };

  class Flags;

  class Cursor {
    State state;
    ILexer &lexer;
    IReactor &reactor;

    // Traverse the token stream discarding unwanted tokens as per
    // `State::Filter` flags, once a wanted one is found, it is set
    // and return.
    void set_next_token() {
      for (;;) {
        token::Token token = lexer.lex();
        if (not state.ignore(token)) {
          state.current = token;
          break;
        }
      }
    }

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
    State::Filter flags(State::Filter flags, State::Filter mask) {
      auto oflags = state.flags;
      state.flags = static_cast<State::Filter>((oflags & mask) | flags);
      return oflags;
    }

    Flags with();
    Flags with(State::Filter flags);
    Flags with(State::Filter flags, State::Filter mask);

    void drain_program() {
      // Drain the lexer
      lexer.drain();
      // Set current to an kind `Eot`
      state.current = lexer.lex();
    }

    bool empty() const { return iskind(token::Kind::Eot); }
    token::Token const &peek() const { return state.current; }
    bool iskind(token::Kind kind) const { return state.current.kind == kind; }

    void report(error::Error error) {
      if (reactor.report(error) == OnError::Abort) {
        drain_program();
      } /* else `OnError::Resume` */
    }
    void report(error::Kind kind) { report({kind, state.current}); }
    void report(error::Kind kind, token::Token where) { report({kind, where}); }

    token::Token eat() {
      token::Token eaten = state.current;
      set_next_token();
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
  inline Flags Cursor::with(State::Filter nflags, State::Filter mask) {
    return {*this, flags(nflags, mask)};
  }
} // namespace rattle::parser::internal

