#include "lexer.hpp"
#include <bits/types/error_t.h>
#include <rattle/lexer.hpp>

namespace rattle {
  token::Token Lexer::scan() {
    using Kind = token::Token::Kind;
    while ((lexer::consume_space(state), not state.empty())) {
      switch (state.peek()) {
      case '\\':
        if (state.safe(2)) {
          if (state.match_next('\n')) {
            state.advance();
            state.consume_lexeme();
            break;
          } else {
            state.advance();
            return state.make_token(error_t::globl_invalid_escapee);
          }
        } else {
          state.advance();
          return state.make_token(error_t::globl_escapee_missing);
        }
        // clang-format off
      case ';':
      case '\n':return state.make_token(Kind::Eos);
      case '#': lexer::consume_comment(state); break;
      case '.': return state.make_token(Kind::Dot);
      case ',': return state.make_token(Kind::Comma);
      case '(': return state.make_token(Kind::OpenParen);
      case ')': return state.make_token(Kind::CloseParen);
      case '{': return state.make_token(Kind::OpenBrace);
      case '}': return state.make_token(Kind::CloseBrace);
      case '[': return state.make_token(Kind::OpenBracket);
      case ']': return state.make_token(Kind::CloseBracket);
      case '@': return state.make_token(state.match_next('=') ? Kind::AtEqual: Kind::At);
      case '=': return state.make_token(state.match_next('=') ? Kind::EqualEqual: Kind::Equal);
      case '-': return state.make_token(state.match_next('=') ? Kind::MinusEqual: Kind::Minus);
      case '+': return state.make_token(state.match_next('=') ? Kind::PlusEqual: Kind::Plus);
      case '*': return state.make_token(state.match_next('=') ? Kind::StarEqual: Kind::Star);
      case '/': return state.make_token(state.match_next('=') ? Kind::SlashEqual: Kind::Slash);
      case '&': return state.make_token(state.match_next('=') ? Kind::BitAndEqual: Kind::BitAnd);
      case '|': return state.make_token(state.match_next('=') ? Kind::BitOrEqual: Kind::BitOr);
      case '~': return state.make_token(state.match_next('=') ? Kind::InvertEqual: Kind::Invert);
      case '%': return state.make_token(state.match_next('=') ? Kind::PercentEqual: Kind::Percent);
      case '\'':return lexer::consume_single_string(state);
      case '"': return lexer::consume_multi_string(state);
      case '!':
        return state.match_next('=') ?
            state.make_token(Kind::NotEqual) :
            state.make_token(error_t::malformed_not_equal);
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
        state.advance(); return state.make_token(error_t::invalid_character);
        // clang-format on
      }
    }
    return state.make_token(Kind::Eot);
  }
} // namespace rattle

