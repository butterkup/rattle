#include <iomanip>
#include <ostream>
#include <rattle/lexer/error.hpp>
#include <rattle/token/token.hpp>
#include <rattle/utility.hpp>
#include <sstream>

namespace rattle::utility {
  // Single quote (') will be escaped as it is meant to be used specially
  // to print escaped strings as 'This is \'internal\' string,\n"this one" too'
  std::ostream &operator<<(std::ostream &s, escape e) noexcept {
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

  std::string to_string(escape e) noexcept {
    std::ostringstream buffer;
    buffer << e;
    return buffer.str();
  }
  // For lexing mostly, but hey.
  bool is_binary(int ch) noexcept { return (unsigned)ch - '0' < 2; }
  bool is_octal(int ch) noexcept { return (unsigned)ch - '0' < 8; }
  bool is_decimal(int ch) noexcept { return (unsigned)ch - '0' < 10; }
  bool is_hexadecimal(int ch) noexcept {
    return is_decimal(ch) or ((unsigned)ch | 32) - 'a' < 6;
  }
  bool is_identifier_start_char(int ch) noexcept {
    return ch == '_' or ((unsigned)ch | 32) - 'a' < 26;
  }
  bool is_identifier_body_char(int ch) noexcept {
    return is_decimal(ch) or is_identifier_start_char(ch);
  }
  bool is_whitespace(int ch) noexcept {
    return ch == ' ' or ch == '\t' or ch == '\r';
  }
} // namespace rattle::utility

namespace rattle::token {
  std::string_view to_string(token::Kind kind) noexcept {
    switch (kind) {
#define rattle_undef_forget_token_macro
#define rattle_pp_token_macro(kind, _)                                         \
  case token::Kind::kind:                                                      \
    return #kind;
#include <rattle/token/token_pp.h>
    default:
      return "(token::Kind::Unknown)";
    }
  }

  std::ostream &operator<<(std::ostream &s, Location loc) noexcept {
    s << "Loc{ ";
    if (loc.is_null()) {
      s << "null";
    } else {
      s << "line=" << loc.line << ", column=" << loc.column;
    }
    return s << " }";
  }

  std::ostream &operator<<(std::ostream &s, token::Token const &tk) noexcept {
    return s << "Token{ kind=" << to_string(tk.kind) << ", start=" << tk.start
             << ", end=" << tk.end << ", payload='"
             << utility::escape{tk.lexeme} << "' }";
  }
} // namespace rattle::token

