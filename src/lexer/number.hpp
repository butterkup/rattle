#pragma once

#include "state.hpp"
#include "tokens.hpp"
#include "utility.hpp"

namespace rat::lexer {
  // facade of how numbers are lexed
  namespace number_lexer {
    namespace consume {
      inline std::size_t _if(State& S, bool (* const predicate)(char)) {
        std::string::const_iterator start = S.current;
        while ( not S.empty() and predicate(S.peek()) ) S.advance();
        return S.current - start;
      }

      inline std::size_t b2(State& S) {
        return _if(S, utils::isbinary);
      }
      inline std::size_t b8(State& S) {
        return _if(S, utils::isoctal);
      }
      inline std::size_t b10(State& S) {
        return _if(S, utils::isdecimal);
      }
      inline std::size_t b16(State& S) {
        return _if(S, utils::ishexadecimal);
      }
    }

    Token::Kind consume_int_float(State& S) {
      auto kind = Token::Kind::Decimal;
      consume::b10(S);

      if ( not S.empty() and S.match('.') )
        if ( not consume::b10(S) ) kind = (
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
        if ( not consume::b10(S) ) kind = (
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
          return (S.advance(), consume::b16(S)) ? Token::Kind::Hexadecimal : (
            S.report_error("At least one hexadecimal value is expected after '0x'"),
            Token::Kind::Error
            );
        case 'o':
          return (S.advance(), consume::b8(S)) ? Token::Kind::Octal : (
            S.report_error("At least one octal value is expected after '0o'"),
            Token::Kind::Error
            );
        case 'b':
          return (S.advance(), consume::b2(S)) ? Token::Kind::Binary : (
            S.report_error("At least one binary value is expected after '0b'"),
            Token::Kind::Error
            );
        }
      return consume_int_float(S);
    }
  }

  using number_lexer::consume_number;
}
