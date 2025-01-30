#pragma once

#ifndef RATTLE_SOURCE_ONLY
#include "category.hpp"
#include <deque>
#include <string>
#endif

namespace rattle {
  namespace lexer {
    enum class error_t {
#define ERROR_MACRO(error) error,
#define ERROR_INCLUDE LEXER_ERROR
#include "error_macro.hpp"
    };

    struct Location {
      std::size_t line, column;
      std::string_view::iterator offset;
    };

    struct Error {
      error_t type;
      Location start, end;
      Error(error_t type, Location const &start, Location const &end)
        : type(type), start(start), end(end) {}
      std::string_view payload() const {
        return { start.offset, end.offset };
      }
    };

    enum class token_kind_t {
#define TK_MACRO(kind, _) kind,
#include "token_macro.hpp"
    };

    struct Token {
      using Kind = token_kind_t;
      Kind kind;
      Location start, end;

      std::string_view payload() const {
        return { start.offset, end.offset };
      }
    };

    std::string_view token_content(std::string const &content,
                                   std::size_t start, std::size_t end);
    const char *to_string(Token::Kind kind);
    const char *to_string(error_t error);

    class State {
      std::string_view content;
      std::deque<Error> &errors;
      Location curloc, lexloc;
      std::string_view::iterator iter, lexstart;

      char advance_loc(char ch);

    public:
      State(std::string_view content, std::deque<Error> &errors)
        : content(content), errors(errors), curloc{1, 0, 0}, lexloc{1, 0, 0},
          iter(content.begin()), lexstart(iter) {}
      State(State &&) = delete;
      State(State const &) = delete;

      void reset();
      bool empty() const;
      char advance();
      void consume_lexeme();
      char peek(std::ptrdiff_t n = 0) const;
      bool match(char expected);
      bool match_next(char expected);
      Location lexeme_location() const;
      Location current_location() const;
      Token make_token(Token::Kind kind);
      Token make_token(error_t error);
      void report(error_t error);
      void report(error_t error, Location start);
      void report(error_t error, Location start, Location end);
      std::size_t max_safe() const;
      bool safe(std::size_t n = 1) const;
      std::string_view lexeme() const;

      friend class Lexer;
    };
  } // namespace lexer

  class Lexer {
    lexer::State state;

  public:
    std::deque<lexer::Error> errors;
    Lexer(std::string_view content): state(content, errors), errors() {}
    lexer::Token scan();
  };
} // namespace rattle

