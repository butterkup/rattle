#pragma once

#include "state.hpp"
#include "tokens.hpp"
#include "utility.hpp"

namespace rat::lexer {
  // facade of how numbers are lexed
  namespace number_lexer {
    namespace consume {
      inline auto _if(State& S, bool (* const predicate)(char)) {
        std::string::iterator start = S.current;
        while ( not S.empty() and predicate(S.peek()) ) S.advance();
        return S.current - start;
      }

      inline auto b2(State& S) {
        return _if(S, utils::isbinary);
      }
      inline auto b8(State& S) {
        return _if(S, utils::isoctal);
      }
      inline auto b10(State& S) {
        return _if(S, utils::isdecimal);
      }
      inline auto b16(State& S) {
        return _if(S, utils::ishexadecimal);
      }
    }

    namespace ns {
      std::pair<Token::Kind, std::size_t>
        number_separator(State& S, bool(* const checker)(char), Token::Kind kind, const char* msg) {
        std::size_t total{ 0 };
        for ( ; not S.empty();) {
          total += consume::_if(S, checker);
          if ( S.safe(2) and S.peek() == '_' ) {
            if ( checker(S.peek(1)) ) S.erase();
            else if ( S.peek(1) == '_' ) kind = (S.advance(), S.report_error(msg,
              "Make sure all characters in the literals belong to the base of the literal "
              "and there is atmost one separator between each."
            ), Token::Kind::Error);
            else break;
          } else break;
        }
        return { kind, total };
      }

      auto b2(State& S) {
        return number_separator(
          S, utils::isbinary,
          Token::Kind::Binary,
          "Invalid binary literal."
        );
      }
      auto b8(State& S) {
        return number_separator(
          S, utils::isoctal,
          Token::Kind::Octal,
          "Invalid octal literal."
        );
      }
      auto b10(State& S) {
        return number_separator(
          S, utils::isdecimal,
          Token::Kind::Decimal,
          "Invalid decimal literal."
        );
      }
      auto b16(State& S) {
        return number_separator(
          S, utils::ishexadecimal,
          Token::Kind::Hexadecimal,
          "Invalid hexadecimal literal."
        );
      }
    }

    Token::Kind consume_int_float(State& S) {
      auto kind = Token::Kind::Decimal;
      ns::b10(S);

      if ( not S.empty() and S.match('.') )
        if ( not ns::b10(S).second ) kind = (
          S.report_error(
            "Expected at least one digit after decimal point.",
            "Put at least one digit after the decimal point."
          ), Token::Kind::Error
          ); else kind = Token::Kind::Floating;

      if ( not S.empty() and (S.match('e') or S.match('E')) ) {
        kind = (
          not S.empty() and (S.match('+') or S.match('-')),
          Token::Kind::Floating
          );
        if ( not ns::b10(S).second ) kind = (
          S.report_error(
            "Missing exponent value.",
            "Put at least one digit in the exponent section."
          ), Token::Kind::Error
          );
      }
      return kind;
    }

    Token::Kind consume_number(State& S) {
      if ( S.match('0') and not S.empty() )
        switch ( S.peek() ) {
        case 'x':
          return (S.advance(), ns::b16(S).second) ? Token::Kind::Hexadecimal : (
            S.report_error("At least one hexadecimal value is expected after '0x'"),
            Token::Kind::Error
            );
        case 'o':
          return (S.advance(), ns::b8(S)).second ? Token::Kind::Octal : (
            S.report_error("At least one octal value is expected after '0o'"),
            Token::Kind::Error
            );
        case 'b':
          return (S.advance(), ns::b2(S)).second ? Token::Kind::Binary : (
            S.report_error("At least one binary value is expected after '0b'"),
            Token::Kind::Error
            );
        }
      return consume_int_float(S);
    }
  }

  using number_lexer::consume_number;
}
