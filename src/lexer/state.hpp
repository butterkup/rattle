#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <queue>
#include "utility.hpp"
#include "tokens.hpp"


namespace rat::lexer {
  struct State {
    Config& cfg;
    std::string::const_iterator
      current, start_lexeme, start_line;
    std::string::const_iterator const
      _start, _end;
    std::size_t _start_line, _last_line;

    State() = delete;
    explicit State(Config& cfg)
      : cfg{ cfg },
      current{ cfg.source.content.cbegin() },
      start_lexeme{ current },
      start_line{ current },
      _start{ current },
      _end{ cfg.source.content.cend() },
      _start_line{ 1UL },
      _last_line{ _start_line } { }

    Location location() const {
      return {
        .start_line = this->_start_line,
        .last_line = this->_last_line,
        .start_column = this->start_lexeme - this->start_line,
        .end_column = this->current - this->start_line,
        .view_start = this->start_lexeme - this->_start,
        .view_end = this->current - this->_start
      };
    }

    inline bool empty() const {
      return this->current == this->_end;
    }

    inline std::size_t max_safe() const {
      return (this->_end - this->current);
    }

    inline bool safe(std::size_t const count) const {
      return this->max_safe() >= count;
    }

    char advance() {
      char advanced = *this->current++;
      if ( advanced == '\n' ) {
        this->cfg.source.lines.emplace_back(
          this->start_line, this->current
        );
        this->start_line = this->current;
        this->_last_line++;
      }
      return advanced;
    }

    void consume() {
      this->start_lexeme = this->current;
      this->_start_line = this->_last_line;
    }

    inline char peek(std::size_t n = 0) const {
      return *(this->current + n);
    }

    inline bool match(char expected) {
      return this->peek() == expected ?
        (this->advance(), true) : false;
    }

    inline bool match_next(char expected) {
      return (this->advance(), this->match(expected));
    }

    inline std::string_view lexeme() const {
      return { this->start_lexeme, this->current };
    }

    void report_error(std::string_view msg, std::string_view fix = "") {
      this->cfg.errors.emplace(msg, fix, this->location());
    }

    Token make_token(Token::Kind kind) {
      Token token{ kind, this->lexeme(), this->location() };
      return (this->consume(), token);
    }
  };
}
