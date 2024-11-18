#include "lexer.hpp"
#include <functional>
#include <rattle/lexer.hpp>

namespace rattle::lexer {
  static bool fltend(char ch) {
    switch (ch) {
    case 'e':
    case 'E':
      return true;
    default:
      return numend(ch);
    }
  }
  template <error_t error, char separator>
  static std::size_t
  number_seq(State &state, std::function<bool(char)> const &pred,
             Token::Kind &kind,
             std::function<bool(char)> const &isatend = numend) {
    std::size_t count = 0;
    while (not state.empty()) {
      if (pred(state.peek())) {
        state.advance();
        count++;
      } else if (state.peek() == separator) {
        auto loc = state.current_location();
        if (state.safe(2)) {
          if (state.peek(1) == separator) {
            state.advance();
            state.report(error_t::repeated_numeric_separator, loc);
            kind = Token::Kind::Error;
          } else if (pred(state.peek(1))) {
            state.advance_erase();
          }
        } else {
          state.advance();
          state.report(error_t::trailing_numeric_separator, loc);
          kind = Token::Kind::Error;
        }
      } else if (isatend(state.peek())) {
        break;
      } else {
        auto loc = state.current_location();
        state.advance();
        state.report(error, loc);
        kind = Token::Kind::Error;
      }
    }
    return count;
  }
  template <char separator> static Token consume_number_helper(State &state) {
    Token::Kind kind = Token::Kind::Decimal;
    if (state.peek() == '0') {
      state.advance();
      if (state.safe()) {
        switch (state.peek()) {
        case 'x':
          state.advance();
          kind = Token::Kind::Hexadecimal;
          if (not number_seq<error_t::invalid_hex_character, separator>(
                state, ishex, kind)) {
            state.report(error_t::empty_hex_literal);
            kind = Token::Kind::Error;
          }
          return state.make_token(kind);
        case 'o':
          state.advance();
          kind = Token::Kind::Octal;
          if (not number_seq<error_t::invalid_oct_character, separator>(
                state, isoct, kind)) {
            state.report(error_t::empty_oct_literal);
            kind = Token::Kind::Error;
          }
          return state.make_token(kind);
        case 'b':
          state.advance();
          kind = Token::Kind::Binary;
          if (not number_seq<error_t::invalid_bin_character, separator>(
                state, isbin, kind)) {
            state.report(error_t::empty_bin_literal);
            kind = Token::Kind::Error;
          }
          return state.make_token(kind);
        default:
          if (number_seq<error_t::invalid_dec_character, separator>(
                state, isdec, kind, fltend)) {
            state.report(error_t::leading_zero_in_decimal);
            kind = Token::Kind::Error;
          }
        }
      }
    }
    number_seq<error_t::invalid_dec_character, separator>(state, isdec, kind,
                                                          fltend);
    if (not state.empty()) {
      bool is_float = false;
      if (state.peek() == '.') {
        is_float = true;
        auto loc = state.current_location();
        state.advance();
        if (not number_seq<error_t::invalid_dec_character, separator>(
              state, isdec, kind, fltend)) {
          state.report(error_t::dangling_decimal_point, loc);
          kind = Token::Kind::Error;
        }
      }
      if (state.safe() and (state.peek() == 'e' or state.peek() == 'E')) {
        is_float = true;
        auto loc = state.current_location();
        state.advance();
        if (state.safe() and (state.peek() == '+' or state.peek() == '-')) {
          state.advance();
        }
        if (not number_seq<error_t::invalid_dec_character, separator>(
              state, isdec, kind)) {
          state.report(error_t::missing_exponent_after_e, loc);
          kind = Token::Kind::Error;
        }
      }
      if (is_float and kind != Token::Kind::Error) {
        kind = Token::Kind::Floating;
      }
    }
    return state.make_token(kind);
  }

  Token consume_number(State &state) {
    return consume_number_helper<'_'>(state);
  }
} // namespace rattle::lexer

