#include "utility.hpp"
#include <cctype>
#include <lexer/lexer.hpp>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace rattle::lexer::internal {
  namespace utility {
    bool is_binary(char ch) { return ch == '0' || ch == '1'; }
    bool is_octal(char ch) { return '0' <= ch && ch <= '7'; }
    bool is_decimal(char ch) { return '0' <= ch && ch <= '9'; }
    bool is_whitespace(char ch) {
      return ch == ' ' || ch == '\t' || ch == '\r';
    }
    bool is_hexadecimal(char ch) {
      return is_decimal(ch) || ('a' <= ch && ch <= 'f') ||
             ('A' <= ch && ch <= 'F');
    }
    bool is_identifier_start_char(char ch) {
      return ch == '_' || std::isalpha(ch);
    }
    bool is_identifier_body_char(char ch) {
      return ch == '_' || std::isalnum(ch);
    }
  } // namespace utility

  // Mapping from keyword string to token_kind_t
  static std::unordered_map<std::string_view, token_kind_t> const keywords{
#define rattle_undef_forget_token_macro
#define TOKEN_MACRO(Kind, keyword) {keyword, token_kind_t::Kind},
#include <lexer/require_pp/keywords.h>
#include <lexer/require_pp/undefine.h>
  };

  // consume names; keywords and variable names.
  token_t cursor_t::consume_identifier() {
    base.eat_while(utility::is_identifier_body_char);
    try {
      return base.make_token(keywords.at(base.get_lexeme()));
    } catch (std::out_of_range &) {
      return base.make_token(token_kind_t::Identifier);
    }
  }

  token_t cursor_t::consume_whitespace() {
    base.eat_while(utility::is_whitespace);
    return base.make_token(token_kind_t::Whitespace);
  }

  token_t cursor_t::consume_comment() {
    base.eat_while([](char ch) { return ch != '\n'; });
    return base.make_token(token_kind_t::Pound);
  }

  std::string_view to_string(token_kind_t kind) {
    switch (kind) {
#define rattle_undef_forget_token_macro
#define TOKEN_MACRO(Kind, _)                                                   \
  case token_kind_t::Kind:                                                     \
    return #Kind;
#include <lexer/tokens_pp.h>
    default:
      return "(token_kind_t::Unknown)";
    }
  }

  std::string_view to_string(error_kind_t kind) {
    switch (kind) {
#define ERROR_MACRO(Kind)                                                      \
  case error_kind_t::Kind:                                                     \
    return #Kind;
#include <lexer/errors_pp.h>
    default:
      return "(error_kind_t::Unknown)";
    }
  }

  std::ostream &operator<<(std::ostream &s, location_t const &loc) {
    return s << "Loc{ line=" << loc.line << ", column=" << loc.column << " }";
  }

  std::ostream &operator<<(std::ostream &s, token_t const &tk) {
    return s << "Token{ kind=" << to_string(tk.kind) << ", start=" << tk.start
             << ", end=" << tk.end << ", payload='" << tk.lexeme << "' }";
  }

  std::ostream &operator<<(std::ostream &s, error_t const &err) {
    return s << "Error{ kind=" << to_string(err.kind) << ", start=" << err.start
             << ", end=" << err.end << ", payload='" << err.lexeme << "' }";
  }
} // namespace rattle::lexer::internal

