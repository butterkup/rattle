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
    std::string::iterator
      current, start_lexeme,
      start_line, _start, _end;
    std::size_t _start_line, _current_line;

    State() = delete;
    explicit State(Config& cfg)
      : cfg{ cfg },
      current{ cfg.source.content.begin() },
      start_lexeme{ current },
      start_line{ current },
      _start{ current },
      _end{ cfg.source.content.end() },
      _start_line{ 1UL },
      _current_line{ _start_line } { }

    inline bool empty() const {
      return this->current == this->_end;
    }

    inline std::size_t max_safe() const {
      return this->_end - this->current;
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
        this->_current_line++;
      }
      return advanced;
    }

    void consume() {
      this->start_lexeme = this->current;
      this->_start_line = this->_current_line;
    }

    inline char peek(std::ptrdiff_t n = 0) const {
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

    void report_error_at(SyntaxError const& error) {
      this->cfg.errors.emplace(error);
    }

    void report_error(std::string_view msg, std::string_view fix = "") {
      this->cfg.errors.emplace(
        msg, fix,
        this->start_location(),
        this->current_location()
      );
    }

    Token make_token(Token::Kind kind) {
      Token token{
        kind,
        this->lexeme(),
        this->start_location(),
        this->current_location()
      };
      return (this->consume(), token);
    }

    Location start_location() const {
      auto start_line = this->_start_line == this->_current_line ?
        this->start_line : this->cfg.source.lines.at(this->_start_line - 1).begin;
      return {
        this->_start_line,
        this->start_lexeme - start_line,
        this->start_lexeme - this->_start
      };
    }

    Location current_location() const {
      auto start_line = this->cfg.source.lines.size() ?
        this->cfg.source.lines.back().end : this->_start;
      return {
        this->_current_line,
        this->current - start_line,
        this->current - this->_start
      };
    }

    char erase() {
      char erased = this->peek();
      this->cfg.source.content.erase(this->current);
      return (this->_end--, erased);
    }

    char replace(char new_char) {
      char old_char = this->peek();
      *this->current = new_char;
      return old_char;
    }
  };
}
