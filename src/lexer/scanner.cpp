#include "utility.hpp"
#include <lexer/lexer.hpp>

namespace rattle::lexer::internal {
  static void toplvl_escape(cursor_t &base) {
    base.eat(); // consume escaper: backslash
    if (base.safe()) {
      switch (base.peek()) {
      // Windows CRLF
      case '\r':
        if (not base.match_next('\n')) {
          base.report(error_kind_t::partially_formed_crlf);
        }
        break;
      case '\n':
        base.eat();
        break;
      default:
        base.eat();
        base.report(error_kind_t::invalid_toplvl_escape_sequence);
      }
    } else {
      base.report(error_kind_t::partial_toplvl_escape);
    }
    // Discard
    base.consume_lexeme();
  }

  token_t lexer_t::scan() {
    while (not base.empty()) {
      switch (base.peek()) {
        // clang-format off
        case '\\':  toplvl_escape(base); break;
        case '\'':
        case '"':   return consume_string();
        case '#':   return consume_comment();
        case '\n':  return base.make_token((base.eat(), token_kind_t::Newline));
        case ';':   return base.make_token((base.eat(), token_kind_t::Semicolon));
        case '.':   return base.make_token((base.eat(), token_kind_t::Dot));
        case ',':   return base.make_token((base.eat(), token_kind_t::Comma));
        case '(':   return base.make_token((base.eat(), token_kind_t::OpenParen));
        case ')':   return base.make_token((base.eat(), token_kind_t::CloseParen));
        case '{':   return base.make_token((base.eat(), token_kind_t::OpenBrace));
        case '}':   return base.make_token((base.eat(), token_kind_t::CloseBrace));
        case '[':   return base.make_token((base.eat(), token_kind_t::OpenBracket));
        case ']':   return base.make_token((base.eat(), token_kind_t::CloseBracket));
        case '=':   return base.make_token(base.match_next('=') ?
                      token_kind_t::EqualEqual:
                      token_kind_t::Equal);
        case '-':   return base.make_token(base.match_next('=') ?
                      token_kind_t::MinusEqual:
                      token_kind_t::Minus);
        case '+':   return base.make_token(base.match_next('=') ?
                      token_kind_t::PlusEqual:
                      token_kind_t::Plus);
        case '*':   return base.make_token(base.match_next('=') ?
                      token_kind_t::StarEqual:
                      token_kind_t::Star);
        case '/':   return base.make_token(base.match_next('=') ?
                      token_kind_t::SlashEqual:
                      token_kind_t::Slash);
        case '!':   return base.match_next('=') ?
                      base.make_token(token_kind_t::NotEqual) :
                      base.make_token(error_kind_t::partial_not_equal);
        case '<':   return base.make_token(base.match_next('=') ?
                      token_kind_t::LessEqual :
                      token_kind_t::LessThan);
        case '>':   return base.make_token(base.match_next('=') ?
                      token_kind_t::GreaterEqual :
                      token_kind_t::GreaterThan);
        default:
          if (utility::is_whitespace(base.peek()))
            return consume_whitespace();
          if (utility::is_decimal(base.peek()))
            return consume_number();
          if (utility::is_identifier_start_char(base.peek()))
            return consume_identifier();
          return base.make_token((base.eat(), error_kind_t::unrecognized_toplvl_character));
        // clang-format on
      }
    }
    return base.make_token(token_kind_t::Eot);
  }
} // namespace rattle::lexer::internal
