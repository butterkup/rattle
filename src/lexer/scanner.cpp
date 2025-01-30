#include "lexer.hpp"
#include <rattle/lexer.hpp>

namespace rattle {
  using lexer::error_t, lexer::Token;

  Token Lexer::scan() {
    using Kind = Token::Kind;
    while ((lexer::consume_space(state), not state.empty())) {
      switch (state.peek()) {
      case '\\':
        if (state.safe(2)) {
          if (state.match_next('\n')) {
            state.consume_lexeme();
            break;
          } else {
            state.advance();
            state.advance();
            return state.make_token(error_t::invalid_escape_sequence);
          }
        } else {
          state.advance();
          return state.make_token(error_t::unterminated_escape_sequence);
        }
        // clang-format off
      case '#':   return lexer::consume_comment(state);
      case '\n':  return state.make_token((state.advance(), Kind::Newline));
      case ';':   return state.make_token((state.advance(), Kind::Semicolon));
      case '.':   return state.make_token((state.advance(), Kind::Dot));
      case ',':   return state.make_token((state.advance(), Kind::Comma));
      case ':':   return state.make_token((state.advance(), Kind::Colon));
      case '(':   return state.make_token((state.advance(), Kind::OpenParen));
      case ')':   return state.make_token((state.advance(), Kind::CloseParen));
      case '{':   return state.make_token((state.advance(), Kind::OpenBrace));
      case '}':   return state.make_token((state.advance(), Kind::CloseBrace));
      case '[':   return state.make_token((state.advance(), Kind::OpenBracket));
      case ']':   return state.make_token((state.advance(), Kind::CloseBracket));
      case '@':   return state.make_token(state.match_next('=') ? Kind::AtEqual: Kind::At);
      case '=':   return state.make_token(state.match_next('=') ? Kind::EqualEqual: Kind::Equal);
      case '-':   return state.make_token(state.match_next('=') ? Kind::MinusEqual: Kind::Minus);
      case '+':   return state.make_token(state.match_next('=') ? Kind::PlusEqual: Kind::Plus);
      case '*':   return state.make_token(state.match_next('=') ? Kind::StarEqual: Kind::Star);
      case '/':   return state.make_token(state.match_next('=') ? Kind::SlashEqual: Kind::Slash);
      case '&':   return state.make_token(state.match_next('=') ? Kind::BitAndEqual: Kind::BitAnd);
      case '|':   return state.make_token(state.match_next('=') ? Kind::BitOrEqual: Kind::BitOr);
      case '~':   return state.make_token(state.match_next('=') ? Kind::InvertEqual: Kind::Invert);
      case '%':   return state.make_token(state.match_next('=') ? Kind::PercentEqual: Kind::Percent);
      case '\'':
      case '"': {
        char const quote = state.peek();
        if (state.safe(3) and state.peek() == quote and
            state.peek(1) == quote and state.peek(2) == quote) {
          state.advance(); state.advance();
          return lexer::consume_multi_string(state);
        }
        return lexer::consume_single_string(state);
      }
      case '!':
        return state.match_next('=') ?
            state.make_token(Kind::NotEqual) :
            state.make_token(error_t::incomplete_not_equal_operator);
      case '<':
        return state.make_token(
          state.match_next('<') ?
            (state.match_next('=') ? Kind::LshiftEqual : Kind::Lshift) :
            (state.match_next('=') ? Kind::LessEqual : Kind::Less));
      case '>':
        return state.make_token(
          state.match_next('>') ?
            (state.match_next('=') ? Kind::RshiftEqual : Kind::Rshift) :
            (state.match_next('=') ? Kind::GreaterEqual : Kind::Greater));
      default:
        if (lexer::isdec(state.peek())) return lexer::consume_number(state);
        if (lexer::isalnum(state.peek())) return lexer::consume_identifier(state);
        state.advance(); return state.make_token(error_t::unrecognized_character);
        // clang-format on
      }
    }
    return state.make_token(Kind::Eot);
  }
} // namespace rattle

