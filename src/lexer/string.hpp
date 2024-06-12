#pragma once

#include "state.hpp"
#include "number.hpp"
#include "tokens.hpp"

namespace rat::lexer {
  // facade of how strings are lexed
  namespace string_lexer {
    void consume_if(State& S, const auto predicate) {
      while ( not S.empty() and predicate(S.peek()) ) S.advance();
    }

    // Token::Kind consume_long_string(State& S, const char quote) { }

    Token::Kind consume_short_string(State& S, const char quote) {
      auto predicate = [quote] (char ch) -> bool {
        return not (ch == quote or ch == '\\' or ch == '\n');
      };
      auto kind = Token::Kind::String;
      for ( ;;) {
        consume_if(S, predicate);
        if ( S.empty() )
          return (S.report_error(
            "Unterminated single line string, EOF received before closing quote.",
            "Add the closing quote as the last charcter in the file to close the string."
          ), Token::Kind::Error);
        if ( S.peek() == quote )
          return (S.advance(), kind);
        if ( S.peek() == '\n' )
          return (S.report_error(
            "Single line string was unterminated or spans more than one line"
          ), Token::Kind::Error);
        if ( S.peek() == '\\' ) {
          if ( not S.safe(2) )
            kind = (S.report_error(
              "Missing escaped character.",
              "Remove backslash as it lacks the escapee."
            ), Token::Kind::Error);
          else switch ( S.peek(1) ) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
            S.erase();
            S.replace(S.peek() - '0');
            S.advance();
            break;
          case 'n':
            S.erase();
            S.replace('\n');
            S.advance();
            break;
          case 'f':
            S.erase();
            S.replace('\f');
            S.advance();
            break;
          case 'v':
            S.erase();
            S.replace('\v');
            S.advance();
            break;
          case '\'':
          case '"':
          case '\\':
            S.erase();
            S.advance();
            break;
          case 'x': {
            if ( S.safe(4) ) {
              auto ishx = utils::ishexadecimal;
              if ( ishx(S.peek(2)) and ishx(3) ) {
                char ch = (S.peek(2) << 8) | S.peek(3);
                S.erase(); S.erase(); S.erase();
                S.replace(ch);
              } else  kind = (
                S.report_error(
                  "Invalid escape sequence, '\\xXX'",
                  "Make sure the two characters after '\\x' "
                  "literal are hexadecimal values."
                ), Token::Kind::Error
                );
            } else kind = (
              S.advance(),
              S.advance(),
              S.report_error(
                "Invalid escape sequence for '\\x', expected '\\xXX'."
              ), Token::Kind::Error
              );
          }
          default: kind = (
            S.advance(),
            S.advance(),
            S.report_error(
              "Unexpected escaped character."
            ), Token::Kind::Error
            );
          }
        }
      }
    }

    Token::Kind consume_string(State& S) {
      return consume_short_string(S, S.advance());
    }
  }

  using string_lexer::consume_string;
}
