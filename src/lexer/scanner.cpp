#include "utility.hpp"
#include <rattle/lexer/lexer.hpp>

namespace rattle::lexer::internal {
  static void toplvl_escape(Cursor &base) {
    base.eat(); // consume escaper: backslash
    if (base.safe()) {
      switch (base.peek()) {
      // Windows CRLF
      case '\r':
        if (not base.match_next('\n')) {
          base.report(error::Kind::partially_formed_crlf);
        }
        break;
      case '\n':
        base.eat();
        break;
      default:
        base.eat();
        base.report(error::Kind::invalid_toplvl_escape_sequence);
      }
    } else {
      base.report(error::Kind::partial_toplvl_escape);
    }
    // Discard
    base.flush_lexeme();
  }

  token::Token Lexer::scan() {
    while (not base.empty()) {
      switch (base.peek()) {
        // clang-format off
        case '\\':  toplvl_escape(base); break;
        case '\'':
        case '"':   return consume_string();
        case '#':   return consume_comment();
        // Windows CRLF
        case '\r':  if (base.match_next('\n')) {
                      return base.make_token(token::Kind::Newline);
                    } else {
                      return base.make_token(error::Kind::partially_formed_crlf);
                    }
        case '\n':  return base.make_token((base.eat(), token::Kind::Newline));
        case ';':   return base.make_token((base.eat(), token::Kind::Semicolon));
        case '.':   return base.make_token((base.eat(), token::Kind::Dot));
        case ',':   return base.make_token((base.eat(), token::Kind::Comma));
        case '(':   return base.make_token((base.eat(), token::Kind::OpenParen));
        case ')':   return base.make_token((base.eat(), token::Kind::CloseParen));
        case '{':   return base.make_token((base.eat(), token::Kind::OpenBrace));
        case '}':   return base.make_token((base.eat(), token::Kind::CloseBrace));
        case '[':   return base.make_token((base.eat(), token::Kind::OpenBracket));
        case ']':   return base.make_token((base.eat(), token::Kind::CloseBracket));
        case '=':   return base.make_token(base.match_next('=') ?
                      token::Kind::EqualEqual:
                      token::Kind::Equal);
        case '-':   return base.make_token(base.match_next('=') ?
                      token::Kind::MinusEqual:
                      token::Kind::Minus);
        case '+':   return base.make_token(base.match_next('=') ?
                      token::Kind::PlusEqual:
                      token::Kind::Plus);
        case '*':   return base.make_token(base.match_next('=') ?
                      token::Kind::StarEqual:
                      token::Kind::Star);
        case '/':   return base.make_token(base.match_next('=') ?
                      token::Kind::SlashEqual:
                      token::Kind::Slash);
        case '!':   return base.match_next('=') ?
                      base.make_token(token::Kind::NotEqual) :
                      base.make_token(error::Kind::partial_not_equal);
        case '<':   return base.make_token(base.match_next('=') ?
                      token::Kind::LessEqual :
                      token::Kind::LessThan);
        case '>':   return base.make_token(base.match_next('=') ?
                      token::Kind::GreaterEqual :
                      token::Kind::GreaterThan);
        default:
          if (utility::is_whitespace(base.peek()))
            return consume_whitespace();
          if (utility::is_decimal(base.peek()))
            return consume_number();
          if (utility::is_identifier_start_char(base.peek()))
            return consume_identifier();
          return base.make_token((base.eat(), error::Kind::unrecognized_toplvl_character));
        // clang-format on
      }
    }
    return base.make_token(token::Kind::Eot);
  }
} // namespace rattle::lexer::internal
