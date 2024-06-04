#ifndef _rat_tokens
#define _rat_tokens

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
      case Floating:
      case Binary:
      case Decimal:
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
    const Kind kind;
    const std::string_view content;
    const std::size_t line, column, offset;
  };
}

#endif //_rat_tokens
