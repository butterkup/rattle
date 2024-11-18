#pragma once

#include "category.hpp"
#include <deque>
#include <string>

namespace rattle {
  namespace lexer {
    enum class error_t {
#define ERROR_MACRO(error) error,
#define ERROR_INCLUDE LEXER_ERROR
#include "error_macro.hpp"
    };

    struct Location {
      std::size_t line, column, offset;
    };
    struct Error {
      error_t type;
      Location start, end;
      Error(error_t type, Location const &start, Location const &end)
        : type(type), start(start), end(end) {}
    };
    namespace __detail {
      enum class token_kind {
#define TK_MACRO(kind, _) kind,
#include "token_macro.hpp"
      };
    } // namespace __detail

    struct Token {
      using Kind = __detail::token_kind;
      Kind kind;
      Location start, end;
    };

    std::string_view token_content(std::string const &content,
                                   std::size_t start, std::size_t end);
    const char *to_string(lexer::Token::Kind kind);
    const char *to_string(lexer::error_t error);

    class State {
      std::string &content;
      std::deque<Error> &errors;
      Location curloc, lexloc;
      std::string::iterator iter;

      char advance_loc(char ch);

    public:
      State(std::string &content, std::deque<Error> &errors)
        : content(content), errors(errors), curloc{1, 0, 0}, lexloc{1, 0, 0},
          iter(content.begin()) {}
      void reset();
      bool empty() const;
      char advance();
      void consume_lexeme();
      char peek(std::ptrdiff_t n = 0) const;
      bool match(char expected);
      bool match_next(char expected);
      void advance_erase();
      void advance_replace(char new_char);
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
    std::string content;
    lexer::State state;

  public:
    std::deque<lexer::Error> errors;

    Lexer(std::string const &_content)
      : content(_content), state(content, errors), errors() {}
    Lexer(std::string &&_content)
      : content(std::move(_content)), state(content, errors), errors() {}
    lexer::Token scan();
    std::string reset(std::string const &content);
    std::string reset(std::string &&content = "");
    std::string const &get_content() const;
  };

} // namespace rattle

