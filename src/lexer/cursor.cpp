#include "utility.hpp"
#include <cctype>
#include <iomanip>
#include <lexer/lexer.hpp>
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
  } // namespace internal

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
    s << "Loc{ ";
    if (loc.is_null()) {
      s << "NULL";
    } else {
      s << "line=" << loc.line << ", column=" << loc.column;
    }
    return s << " }";
  }

  std::ostream &operator<<(std::ostream &s, token_t const &tk) {
    return s << "Token{ kind=" << to_string(tk.kind) << ", start=" << tk.start
             << ", end=" << tk.end << ", payload='" << escape_t{tk.lexeme}
             << "' }";
  }

  std::ostream &operator<<(std::ostream &s, error_t const &err) {
    return s << "Error{ kind=" << to_string(err.kind) << ", start=" << err.start
             << ", end=" << err.end << ", payload='" << escape_t{err.lexeme}
             << "' }";
  }

  // Single quote (') will be escaped as it is meant to be used specially
  // to print escaped strings as 'This is \'internal\' string,\n"this one" too'
  std::ostream &operator<<(std::ostream &s, escape_t const &e) {
    // We must reset to callers settings
    auto flags = s.flags();
    // For printing the escapes like \x23 and \xad
    s << std::hex;
    for (int ch : e.view) {
      switch (ch) {
      case '\'':
        s << "\\'";
        break;
      case '\n':
        s << "\\n";
        break;
      case '\r':
        s << "\\r";
        break;
      case '\f':
        s << "\\f";
        break;
      case '\v':
        s << "\\v";
        break;
      case '\t':
        s << "\\t";
        break;
      case '\0':
        s << "\\0";
        break;
      case '\a':
        s << "\\a";
        break;
      case '\b':
        s << "\\b";
        break;
      default:
        if (std::isprint(ch)) {
          s << static_cast<char>(ch);
        } else {
          s << "\\x" << std::setw(2) << std::setfill('0') << ch << '\n';
        }
      }
    }
    s.flags(flags);
    return s;
  }
} // namespace rattle::lexer

