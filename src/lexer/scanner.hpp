#pragma once

#include "state.hpp"
#include "utility.hpp"
#include "string.hpp"
#include "number.hpp"
#include <stdexcept>

namespace rat::lexer {
  void consume_comment(State& S) {
    while ( not S.empty() and S.peek() != '\n' )
      S.advance();
    // Comments are wrapped as token HashTag
  }

  void consume_space(State& S) {
    while ( not S.empty() and utils::isspace(S.peek()) )
      S.advance();
    S.consume(); // discard them spaces
  }

  Token::Kind consume_identifier(State& S) {
    while ( not S.empty() and utils::isidchar(S.peek()) )
      S.advance();
    try {
      return Keywords.at(S.lexeme());
    } catch ( std::out_of_range ) {
      return Token::Kind::Identifier;
    }
  }

  Token scan(State& S) {
    using enum Token::Kind;
    Location location_start;
    while ( (consume_space(S), not S.empty()) ) {
      location_start = S.current_location();
      switch ( S.peek() ) {
      case '#': return S.make_token((consume_comment(S), HashTag));
      case '>':
        return S.match_next('>') ?
          S.make_token(S.match_next('=') ? RshiftEqual : Rshift) :
          S.make_token(S.match_next('=') ? GreaterEqual : Greater);
      case '<':
        return S.match_next('<') ?
          S.make_token(S.match_next('=') ? LshiftEqual : Lshift) :
          S.make_token(S.match_next('=') ? LessEqual : Less);
      case '+': return S.make_token(S.match_next('=') ? PlusEqual : Plus);
      case '*': return S.make_token(S.match_next('=') ? StarEqual : Star);
      case '-': return S.make_token(S.match_next('=') ? MinusEqual : Minus);
      case '/': return S.make_token(S.match_next('=') ? SlashEqual : Slash);
      case '=': return S.make_token(S.match_next('=') ? EqualEqual : Equal);
      case '|': return S.make_token(S.match_next('=') ? BitOrEqual : BitOr);
      case '&': return S.make_token(S.match_next('=') ? BitAndEqual : BitAnd);
      case '~': return S.make_token(S.match_next('=') ? InvertEqual : Invert);
      case '@': return S.make_token(S.match_next('=') ? AtEqual : At);
      case '!': return S.make_token(
        S.match_next('=') ? NotEqual : (
          S.report_error(
            location_start,
            "Malformed operator '!=', '!' must be followed by '='.",
            "Add '=' after '!' or did you mean 'not', the keyword?"
          ), Error
          ));
      case '%': return S.make_token(S.match_next('=') ? PercentEqual : Percent);
      case '.': return S.make_token((S.advance(), Dot));
      case ',': return S.make_token((S.advance(), Comma));
      case '[': return S.make_token((S.advance(), OpenBracket));
      case ']': return S.make_token((S.advance(), CloseBracket));
      case '(': return S.make_token((S.advance(), OpenParen));
      case ')': return S.make_token((S.advance(), CloseParen));
      case '{': return S.make_token((S.advance(), OpenBrace));
      case '}': return S.make_token((S.advance(), CloseBrace));
      case ';': return S.make_token((S.advance(), Semicolon));
      case '\'':
      case '"': return S.make_token(consume_string(S));
      default:
        if ( utils::isdecimal(S.peek()) )
          return S.make_token(consume_number(S));
        else if ( utils::isidchar(S.peek()) )
          return S.make_token(consume_identifier(S));
        else return S.make_token((
          location_start = S.current_location(),
          S.advance(),
          S.report_error(
            location_start,
            "Unexpected character."
          ), Error
          ));
      }
    }
    return S.make_token(Eot);
  }
}
