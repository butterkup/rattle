#pragma once

#include <string>
#include <string_view>
#include "utility.hpp"
#include "tokens.hpp"

namespace rat::lexer {
  struct State {
    Config& cfg;

    std::string::iterator
      current, start_lexeme,
      start_line, start_src, end_src,
      start_line_lexeme;
    std::size_t lexeme_line;

    State() = delete;
    State(Config& cfg)
      : cfg(cfg),
      current(cfg.source.content.begin()),
      start_lexeme(current),
      start_line(current),
      start_src(current),
      end_src(cfg.source.content.end()),
      start_line_lexeme(current),
      lexeme_line(0) { }

    inline bool empty() const noexcept {
      return this->current == this->end_src;
    }

    char advance() noexcept {
      char advanced = *this->current++;
      if ( advanced == '\n' ) {
        this->cfg.source.lines.emplace_back(
          this->start_line, this->current
        );
        this->start_line = this->current;
      }
      return advanced;
    }

    void consume() noexcept {
      this->start_lexeme = this->current;
      this->start_line_lexeme = this->start_line;
      this->lexeme_line = this->cfg.source.lines.size();
    }

    inline std::size_t max_safe() const noexcept {
      return this->end_src - this->current;
    }

    inline bool safe(std::size_t n = 1) const noexcept {
      return this->max_safe() >= n;
    }

    inline char peek(std::ptrdiff_t n = 0) const noexcept {
      return *(this->current + n);
    }

    inline bool match(char expected) noexcept {
      return this->peek() == expected ?
        (this->advance(), true) : false;
    }

    inline bool match_next(char expected) noexcept {
      return (this->advance(), this->match(expected));
    }

    inline std::string_view lexeme() const noexcept {
      return { this->start_lexeme, this->current };
    }

    char erase() noexcept {
      char erased = *this->current;
      this->cfg.source.content.erase(this->current);
      return (this->end_src--, erased);
    }

    char replace(char new_char) noexcept {
      char old_char = *this->current;
      *this->current = new_char;
      return old_char;
    }

    inline Location lexeme_location() const noexcept {
      return {
        this->lexeme_line,
        this->start_lexeme - this->start_line_lexeme,
        this->start_lexeme - this->start_src
      };
    }

    inline Location current_location() const noexcept {
      return {
        this->cfg.source.lines.size(),
        this->current - this->start_line,
        this->current - this->start_src,
      };
    }

    Token make_token(Token::Kind kind) noexcept {
      Token token{
        kind, this->lexeme(),
        this->lexeme_location(),
        this->current_location()
      };
      return (this->consume(), token);
    }

    inline void report_error_at(Location start, Location end, std::string_view msg, std::string_view fix = "") noexcept {
      this->cfg.errors.emplace(msg, fix, start, end);
    }

    inline void report_error(std::string_view msg, std::string_view fix = "") noexcept {
      this->report_error_at(
        this->lexeme_location(),
        this->current_location(),
        msg, fix
      );
    }
  };
}
