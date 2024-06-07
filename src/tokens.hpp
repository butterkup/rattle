#ifndef _rat_tokens
#define _rat_tokens

#include "config.hpp"
#include <string_view>

namespace rat {
  namespace _impl {
    enum class token_kind {
#define TOKEN_KIND(kind) kind,
# include "token_kind.hpp"
#undef TOKEN_KIND
    };
    const char* to_string(token_kind kind) {
      using enum token_kind;
      switch ( kind ) {
#define TOKEN_KIND(kind) case kind: return #kind;
# include "token_kind.hpp"
#undef TOKEN_KIND
      default: return "<Unknown TokenKind>";
      }
    }
    bool is_number(token_kind kind) {
      using enum token_kind;
      switch ( kind ) {
      case Octal:
      case Binary:
      case Decimal:
      case Floating:
      case Hexadecimal: return true;
      default: return false;
      }
    }
    bool is_string(token_kind kind) {
      using enum token_kind;
      switch ( kind ) {
      case String:
      case LongString: return true;
      default: return false;
      }
    }
  }

  struct Token {
    using Kind = _impl::token_kind;
    Kind kind;
    std::string_view content;
    Location location;
  };

  std::ostream& operator<<(std::ostream& out, Location const& loc) {
    return out << "Location(line=" << loc.line
      << ", column=" << loc.column
      << ", offset=" << loc.offset << ')';
  }

  std::ostream& operator<<(std::ostream& out, Token const& tk) {
    return out << "Token(kind="
      << _impl::to_string(tk.kind)
      << ", content='" << tk.content
      << "', location=" << tk.location
      << ')';
  }
}

#endif //_rat_tokens
