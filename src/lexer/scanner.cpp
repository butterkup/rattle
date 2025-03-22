#include <rattle/lexer/lexer.hpp>
#include <rattle/token/token.hpp>
#include <rattle/utility.hpp>

namespace rattle::lexer::internal {
  static token::Token toplvl_escape(Cursor &base) noexcept {
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
    return base.make_token(token::kinds::Marker::Escape);
  }

  token::Token Lexer::scan() noexcept {
    while (not cursor.empty()) {
      switch (cursor.peek()) {
        // clang-format off
        case '\'':
        case '"':   return consume_string(token::kinds::String::Normal);
        case '\\':  return toplvl_escape(cursor);
        case '#':   return consume_comment();
        // Windows CRLF
        case '\r':  return cursor.match_next('\n')?
                      cursor.make_token(token::kinds::Marker::Newline):
                      cursor.make_token(error::Kind::partially_formed_crlf);
        case 'r':
        case 'R':   return (cursor.eat(), (cursor.peek() == '"' or cursor.peek() == '\''))?
                      consume_string(token::kinds::String::Raw):
                      consume_identifier();
        case '\n':  return cursor.make_token((cursor.eat(), token::kinds::Marker::Newline));
        case ';':   return cursor.make_token((cursor.eat(), token::kinds::Marker::Semicolon));
        case '(':   return cursor.make_token((cursor.eat(), token::kinds::Marker::OpenParen));
        case ')':   return cursor.make_token((cursor.eat(), token::kinds::Marker::CloseParen));
        case '{':   return cursor.make_token((cursor.eat(), token::kinds::Marker::OpenBrace));
        case '}':   return cursor.make_token((cursor.eat(), token::kinds::Marker::CloseBrace));
        case '[':   return cursor.make_token((cursor.eat(), token::kinds::Marker::OpenBracket));
        case ']':   return cursor.make_token((cursor.eat(), token::kinds::Marker::CloseBracket));
        case '.':   return cursor.make_token((cursor.eat(), token::kinds::Operator::Dot));
        case ',':   return cursor.make_token((cursor.eat(), token::kinds::Operator::Comma));
        case '<':   return cursor.make_token(cursor.match_next('=') ?
                      token::kinds::Operator::LessEqual :
                      token::kinds::Operator::LessThan);
        case '>':   return cursor.make_token(cursor.match_next('=') ?
                      token::kinds::Operator::GreaterEqual :
                      token::kinds::Operator::GreaterThan);
        case '!':   return cursor.match_next('=') ?
                      cursor.make_token(token::kinds::Operator::NotEqual) :
                      cursor.make_token(error::Kind::partial_not_equal);
        case '=':   return cursor.match_next('=') ?
                      cursor.make_token(token::kinds::Operator::EqualEqual):
                      cursor.make_token(token::kinds::Assignment::Equal);
        case '-':   return cursor.match_next('=') ?
                      cursor.make_token(token::kinds::Assignment::MinusEqual):
                      cursor.make_token(token::kinds::Operator::Minus);
        case '+':   return cursor.match_next('=') ?
                      cursor.make_token(token::kinds::Assignment::PlusEqual):
                      cursor.make_token(token::kinds::Operator::Plus);
        case '*':   return cursor.match_next('=') ?
                      cursor.make_token(token::kinds::Assignment::StarEqual):
                      cursor.make_token(token::kinds::Operator::Star);
        case '/':   return cursor.match_next('=') ?
                      cursor.make_token(token::kinds::Assignment::SlashEqual):
                      cursor.make_token(token::kinds::Operator::Slash);
        default:
          if (utility::is_whitespace(cursor.peek()))
            return consume_whitespace();
          if (utility::is_decimal(cursor.peek()))
            return consume_number();
          if (utility::is_identifier_start_char(cursor.peek()))
            return consume_identifier();
          return cursor.make_token((cursor.eat(), error::Kind::unrecognized_toplvl_character));
        // clang-format on
      }
    }
    return cursor.make_token(token::kinds::Token::Eot);
  }
} // namespace rattle::lexer::internal
