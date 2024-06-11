#ifndef _rat_tokens
#define _rat_tokens

#include <ostream>
#include "utility.hpp"
#include <string_view>
#include <unordered_map>

namespace rat::lexer {
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

  const auto Keywords = [] {
    using enum Token::Kind;
    return std::unordered_map<std::string_view, Token::Kind>{
      {"raise", Raise}, { "class", Class }, { "fn", Fn }, { "let", Let }, { "as", As },
      { "import", Import }, { "in", In }, { "not", Not }, { "and", And }, { "or", Or },
      { "try", Try }, { "else", Else }, { "if", If }, { "while", While }, { "for", For },
      { "except", Except }, { "lastly", Lastly }, { "return", Return }, { "with", With },
      { "assert", Assert }, { "from", From }, { "true", True }, { "false", False }
    };
  }();

  std::ostream& operator<<(std::ostream& out, Location const& L) {
    return out << "Location(line=(" << L.start_line << ", " << L.last_line
      << "), column=(" << L.start_column << ", " << L.end_column
      << "), span=(" << L.view_start << ", " << L.view_end << "))";
  }

  std::ostream& operator<<(std::ostream& out, Token const& T) {
    return out << "Token(kind=" << _impl::to_string(T.kind)
      << ", content='" << T.content << "', location="
      << T.location << ')';
  }
}

#endif //_rat_tokens
