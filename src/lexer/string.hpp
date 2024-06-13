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

    char b16t10(char ch) {
      // make sure ch is a valid hexadecimal character: [a-fA-F0-9]
      return ('0' <= ch and ch <= '9') ?
        ch - '0' : 10 + (ch - 'a');
    }

    Token::Kind escape_seqs(State& S, Token::Kind kind = Token::Kind::Eot) {
      using Token::Kind::Error;
      switch ( S.peek(1) ) {
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
          auto const ishx = utils::ishexadecimal;
          if ( ishx(S.peek(2)) and ishx(S.peek(3)) ) {
            char ch = (b16t10(S.peek(2)) << 4) | b16t10(S.peek(3));
            S.erase(); S.erase(); S.erase();
            S.replace(ch); S.advance(); break;
          } else  return (
            S.advance(), S.advance(),
            S.report_error(
              "Invalid escape sequence, '\\xXX'",
              "Make sure the two characters after '\\x' "
              "literal are hexadecimal values."
            ), Error
            );
        } else return (
          S.advance(),
          S.advance(),
          S.report_error(
            "Invalid escape sequence for '\\x', expected '\\xXX'."
          ), Error
          );
      }
      default: return (
        S.advance(),
        S.advance(),
        S.report_error(
          "Unexpected escaped character."
        ), Error
        );
      }
      return kind;
    }

    Token::Kind consume_long_string(State& S, const char quote) {
      auto predicate = [quote] (char ch) -> bool {
        return not (ch == quote or ch == '\\');
      };
      auto kind = Token::Kind::LongString;
      for ( ;;) {
        consume_if(S, predicate);
        if ( S.empty() )
          return (S.report_error(
            "Unterminated multiline string, EOF received before closing quote trio.",
            "Add triple closing quote as the last characters in the file to close the string."
          ), Token::Kind::Error);
        if ( S.match(quote) and S.safe(2) and S.match(quote) and S.match(quote) )
          return kind;
        if ( S.peek() == '\\' )
          if ( not S.safe(2) )
            return (S.report_error(
              "Missing escaped character.",
              "Remove backslash as it lacks the escapee."
            ), Token::Kind::Error);
          else kind = escape_seqs(S, kind);
      }
    }

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
        if ( S.match(quote) )
          return kind;
        if ( S.match('\n') )
          return (S.report_error(
            "Single line string was unterminated or spans more than one line"
          ), Token::Kind::Error);
        kind = escape_seqs(S, kind);
      }
    }

    Token::Kind consume_string(State& S) {
      const char quote = S.advance();
      if ( S.safe(2) and S.peek() == quote and S.peek(1) == quote )
        return (S.advance(), S.advance(), consume_long_string(S, quote));
      return consume_short_string(S, quote);
    }
  }

  using string_lexer::consume_string;
}
