#pragma once

#include "token.hpp"
#include "utility.hpp"
#include <queue>
#include <string>

namespace rattle {
  namespace lexer {
    class State {
      std::string &content;
      Location _cur_loc, _lex_loc;
      std::string::iterator _iter, _lex_start;
      std::queue<Error> &errors;

    public:
      State(std::string &content, std::queue<Error> &errors)
        : content(content), _cur_loc(), _lex_loc(), _iter(content.begin()),
          _lex_start(_iter), errors(errors) {}
      bool empty() const;
      char advance();
      void consume_lexeme();
      char peek(std::ptrdiff_t n = 0) const;
      bool match(char expected);
      bool match_next(char expected);
      void erase_char();
      void replace_char(char new_char);
      Location lexeme_location() const;
      Location current_location() const;
      token::Token make_token(token::Token::Kind kind);
      token::Token make_token(error_t error);
      void report(error_t error);
      void report(error_t error, Location start);
      void report(error_t error, Location start, Location end);
      std::size_t max_safe() const;
      bool safe(std::size_t n = 1) const;
      std::string_view lexeme() const;
    };
  } // namespace lexer
  struct Lexer {
    std::string content;
    std::queue<Error> errors;
    lexer::State state;

    Lexer(std::string const &_content)
      : content(_content), errors(), state(content, errors) {}
    Lexer(std::string &&_content)
      : content(std::move(_content)), errors(), state(content, errors) {}
    token::Token scan();
  };
} // namespace rattle

