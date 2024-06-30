#pragma once

#include "state.hpp"
#include <string>
#include <functional>
#include "tokens.hpp"
#include "utility.hpp"

namespace rat::lexer {
  namespace number_lexer {
    inline auto consume_if(State& S, const std::function<bool(char)> predicate) {
      std::string::iterator start = S.current;
      while ( not S.empty() and predicate(S.peek()) ) S.advance();
      return S.current != start;
    }

    inline bool separator(State& S,
      std::function<bool(char)> predicate,
      std::function<void(State&)> onerror) {
      auto consumer = [&] (State& S) -> bool {
        return consume_if(S, predicate);
      };
      auto start = S.current;
      for ( ; not S.empty();) {
        if ( not consumer(S) ) { onerror(S); break; }
        if ( S.safe(2) and
          S.peek() == '_' and
          predicate(S.peek(1)) )
          S.erase();
      }
      return start != S.current;
    }

    inline Token::Kind check_literals(State& S,
      Token::Kind kind,
      std::function<bool(char)> const predicate,
      std::string_view const invalid_literal_msg,
      std::string_view const fix = "") {
      return separator(S, predicate, [&] (State& S) {
        auto start_loc = S.current_location();
        if ( consume_if(S, utils::isidchar) ) {
          kind = Token::Kind::Error;
          S.report_error_at(start_loc,
            S.current_location(),
            invalid_literal_msg, fix);
        }
      }) ? kind : Token::Kind::Eot;
    }

    Token::Kind base16(State& S) {
      return check_literals(S,
        Token::Kind::Hexadecimal,
        utils::ishexadecimal,
        "Invalid base 16 character literal",
        "did you accidentally join an identifier to a number literal "
        "or use invalid characters in a base 16 number literal"
      );
    }
    Token::Kind base10(State& S) {
      return check_literals(S,
        Token::Kind::Decimal,
        utils::isdecimal,
        "Invalid base 10 character literal",
        "did you accidentally join an identifier to a number literal "
        "or use invalid characters in a base 10 number literal"
      );
    }
    Token::Kind base8(State& S) {
      return check_literals(S,
        Token::Kind::Octal,
        utils::isoctal,
        "Invalid base 8 character literal",
        "did you accidentally join an identifier to a number literal "
        "or use invalid characters in a base 8 number literal"
      );
    }
    Token::Kind base2(State& S) {
      return check_literals(S,
        Token::Kind::Binary,
        utils::isbinary,
        "Invalid base 2 character literal",
        "did you accidentally join an identifier to a number literal "
        "or use invalid characters in a base 2 number literal"
      );
    }


    Token::Kind consume_int_float(State& S) {
      auto kind = base10(S);
      if ( kind != Token::Kind::Decimal ) return kind;
      if ( not S.empty() and S.match('.') ) {
        kind = Token::Kind::Floating;

      }
      return kind;
    }


    Token::Kind base10f(State& S) {
      auto consume = [] (State& S) {
        return separator(S, utils::isdecimal, [] (State&) { });
      };

      auto kind = Token::Kind::Decimal;
      consume(S);

      if ( not S.empty() and S.match('.') )
        if ( not consume(S) ) kind = (
          base10(S),
          S.report_error(
            "Expected at least one digit after decimal point.",
            "Put at least one digit after the decimal point."
          ), Token::Kind::Error);
        else kind = Token::Kind::Floating;

      if ( not S.empty() and (S.match('e') or S.match('E')) ) {
        kind = (
          not S.empty() and (S.match('+') or S.match('-')),
          Token::Kind::Floating
          );
        if ( not consume(S) ) kind = (
          base10(S),
          S.report_error(
            "Missing exponent value.",
            "Put at least one digit in the exponent section."
          ), Token::Kind::Error
          );
      }
      return base10(S) == Token::Kind::Error ?
        Token::Kind::Error : kind;
    }

    Token::Kind consume_number(State& S) {
      auto start_loc = S.current_location();
      const char* msg = nullptr;
      if ( S.safe(2) and S.peek() == '0' ) {
        Token::Kind kind = Token::Kind::Semicolon;
        switch ( S.peek(1) ) {
        case 'b':
          kind = (S.advance(), S.advance(), base2(S));
          msg = "At least one binary literal is expected after 0b";
          break;
        case 'o':
          kind = (S.advance(), S.advance(), base8(S));
          msg = "At least one octal literal is expected after 0o";
          break;
        case 'x':
          kind = (S.advance(), S.advance(), base16(S));
          msg = "At least one hexadecimal literal is expected after 0x";
          break;
        }
        switch ( kind ) {
        case Token::Kind::Eot:
          return (S.report_error(msg), Token::Kind::Error);
        case Token::Kind::Semicolon:
          break;
        default: return kind;
        }
      }
      return base10f(S);
    }
  }

  using number_lexer::consume_number;
}
