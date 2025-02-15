#include "utility.hpp"
#include <cassert>
#include <cctype>
#include <ostream>
#include <rattle/lexer/lexer.hpp>
#include <rattle/utility.hpp>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace rattle::lexer {
  namespace internal {
    namespace utility {
      bool is_binary(char ch) { return (unsigned)ch - '0' < 2; }
      bool is_octal(char ch) { return (unsigned)ch - '0' < 8; }
      bool is_decimal(char ch) { return (unsigned)ch - '0' < 10; }
      bool is_hexadecimal(char ch) {
        return is_decimal(ch) or ((unsigned)ch | 32) - 'a' < 6;
      }
      bool is_identifier_start_char(char ch) {
        return ch == '_' or ((unsigned)ch | 32) - 'a' < 26;
      }
      bool is_identifier_body_char(char ch) {
        return is_decimal(ch) or is_identifier_start_char(ch);
      }
      bool is_whitespace(char ch) {
        return ch == ' ' or ch == '\t' or ch == '\r';
      }
    } // namespace utility

    // Mapping from keyword string to token::Kind
    static std::unordered_map<std::string_view, token::Kind> const keywords{
#define rattle_undef_token_macro
#define rattle_pp_token_macro(kind, keyword) {keyword, token::Kind::kind},
#include <rattle/token/require_pp/keywords.h>
#include <rattle/token/require_pp/undefine.h>
    };

    // consume names; keywords and variable names.
    token::Token Lexer::consume_identifier() {
      base.eat_while(utility::is_identifier_body_char);
      try {
        return base.make_token(keywords.at(base.buffer()));
      } catch (std::out_of_range &) {
        return base.make_token(token::Kind::Identifier);
      }
    }

    token::Token Lexer::consume_whitespace() {
      base.eat_while(utility::is_whitespace);
      return base.make_token(token::Kind::Whitespace);
    }

    token::Token Lexer::consume_comment() {
      base.eat_while([](char ch) { return ch != '\n'; });
      return base.make_token(token::Kind::Pound);
    }

    char Cursor::eat() {
      assert(not empty());
      current.location.column++;
      if (*current.iterator == '\n') {
        current.location.column = token::Location::Valid().column;
        reactor.cache(current.location.line++, {line_start, current.iterator});
        line_start = current.iterator + 1;
      } else if (empty() and line_start != current.iterator) {
        reactor.cache(current.location.line, {line_start, current.iterator});
        line_start = current.iterator;
      }
      return *current.iterator++;
    }
  } // namespace internal

  namespace error {
    std::string_view to_string(Kind kind) {
      switch (kind) {
#define rattle_pp_error_macro(kind)                                            \
  case error::Kind::kind:                                                      \
    return #kind;
#include <rattle/lexer/error_pp.h>
      default:
        return "(error::Kind::Unknown)";
      }
    }

    std::ostream &operator<<(std::ostream &s, Error const &err) {
      return s << "Error{ kind=" << to_string(err.kind)
               << ", start=" << err.start << ", end=" << err.end
               << ", payload='" << utility::escape{err.lexeme} << "' }";
    }
  } // namespace error
} // namespace rattle::lexer
