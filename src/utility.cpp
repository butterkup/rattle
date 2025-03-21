#include <iomanip>
#include <ostream>
#include <rattle/lexer/error.hpp>
#include <rattle/rattle.hpp>
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
  std::string_view to_string(token::kinds::Token kind) noexcept {
    switch (kind) {
#define rattle_pp_token_macro(kind, _)                                         \
  case token::kinds::Token::kind:                                              \
    return #kind;
#include <rattle/token/require_pp/kind.h>
#undef rattle_pp_token_macro
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
    s << "Token{ kind=" << to_string(tk.kind);

    switch (tk.kind) {
    case token::kinds::Token::Eot:
      goto outside;

    case token::kinds::Token::String:
      s << "{ "
        << ((tk.flags & token::kinds::String::Error) ? "Deformed" :
                                                       "Wellformed")
        << ", "
        << ((tk.flags & token::kinds::String::Multiline) ? "Multi" : "Single")
        << "line, "
        << ((tk.flags & token::kinds::String::Raw) ? "Raw" : "Escape") << " }";
      goto outside;

    case token::kinds::Token::Number:
      switch (tk.flags & ~token::kinds::Number::Error) {
#define rattle_pp_token_macro(Kind, _)                                         \
  case token::kinds::Number::Kind:                                             \
    s << "{ status="                                                           \
      << ((tk.flags & token::kinds::Number::Error) ? "Deformed" :              \
                                                     "Wellformed")             \
      << ", kind=" << #Kind << " }";                                           \
    goto outside;
#include <rattle/token/require_pp/kinds/number.h>
#undef rattle_pp_token_macro
      default:
        unreachable();
      }

    case token::kinds::Token::Identifier:
      switch (tk.flags) {
#define rattle_pp_token_macro(Kind, _)                                         \
  case token::kinds::Identifier::Kind:                                         \
    s << "{ " << #Kind << " }";                                                \
    goto outside;
#include <rattle/token/require_pp/kinds/identifier.h>
#undef rattle_pp_token_macro
      default:
        unreachable();
      }

    case token::kinds::Token::Assignment:
      switch (tk.flags) {
#define rattle_pp_token_macro(Kind, _)                                         \
  case token::kinds::Assignment::Kind:                                         \
    s << "{ " << #Kind << " }";                                                \
    goto outside;
#include <rattle/token/require_pp/kinds/assignment.h>
#undef rattle_pp_token_macro
      default:
        unreachable();
      }

    case token::kinds::Token::Marker:
      switch (tk.flags) {
#define rattle_pp_token_macro(Kind, _)                                         \
  case token::kinds::Marker::Kind:                                             \
    s << "{ " << #Kind << " }";                                                \
    goto outside;
#include <rattle/token/require_pp/kinds/marker.h>
#undef rattle_pp_token_macro
      default:
        unreachable();
      }

    case token::kinds::Token::Operator:
      switch (tk.flags) {
#define rattle_pp_token_macro(Kind, _)                                         \
  case token::kinds::Operator::Kind:                                           \
    s << "{ " << #Kind << " }";                                                \
    goto outside;
#include <rattle/token/require_pp/kinds/operator.h>
#undef rattle_pp_token_macro
      default:
        unreachable();
      }

    default:
      unreachable();
    }

  outside:
    return s << ", start=" << tk.start << ", end=" << tk.end << ", payload='"
             << utility::escape{tk.lexeme} << "' }";
  }
} // namespace rattle::token

