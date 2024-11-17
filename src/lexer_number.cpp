#include "lexer.hpp"
#include <functional>
#include <rattle/lexer.hpp>

namespace rattle::lexer {
  template <error_t error, char separator>
  static std::size_t _consume_seq(State &state,
                                  std::function<bool(char)> const &pred,
                                  token::Token::Kind &kind) {
    std::size_t start = state.current_location().offset;
    while (not state.empty()) {
      if (pred(state.peek())) {
        state.advance();
      } else if (state.peek() == separator) {
        auto loc = state.current_location();
        if (state.safe(2)) {
          if (state.peek(1) == separator) {
            state.advance();
            state.report(error_t::multi_adj_num_separators, loc);
            kind = token::Token::Kind::Error;
          } else if (pred(state.peek(1))) {
            state.erase_char();
          }
        } else {
          state.advance();
          state.report(error_t::dangling_num_separator, loc);
          kind = token::Token::Kind::Error;
        }
      } else if (numend(state.peek())) {
        break;
      } else {
        auto loc = state.current_location();
        state.advance();
        state.report(error, loc);
        kind = token::Token::Kind::Error;
      }
    }
    return state.current_location().offset - start;
  }
  template <char separator>
  static token::Token consume_number_helper(State &state) {
    using Kind = token::Token::Kind;
    Kind kind = Kind::Decimal;
    if (state.peek() == '0') {
      state.advance();
      if (state.safe()) {
        switch (state.peek()) {
        case 'x':
          state.advance();
          kind = Kind::Hexadecimal;
          if (not _consume_seq<error_t::expected_hex_literal, separator>(
                state, ishex, kind)) {
            state.report(error_t::empty_hex_literal);
            kind = Kind::Error;
          }
          return state.make_token(kind);
        case 'o':
          state.advance();
          kind = Kind::Octal;
          if (not _consume_seq<error_t::expected_oct_literal, separator>(
                state, isoct, kind)) {
            state.report(error_t::empty_oct_literal);
            kind = Kind::Error;
          }
          return state.make_token(kind);
        case 'b':
          state.advance();
          kind = Kind::Binary;
          if (not _consume_seq<error_t::expected_bin_literal, separator>(
                state, isbin, kind)) {
            state.report(error_t::empty_bin_literal);
            kind = Kind::Error;
          }
          return state.make_token(kind);
        default:
          if (_consume_seq<error_t::expected_dec_literal, separator>(
                state, isdec, kind)) {
            state.report(error_t::zero_prefixed_dec);
            kind = Kind::Error;
          }
        }
      }
    }
    if (not state.empty()) {
      if (state.peek() == '.') {
        auto loc = state.current_location();
        state.advance();
        if (not _consume_seq<error_t::expected_dec_literal, separator>(
              state, isdec, kind)) {
          state.report(error_t::dangling_decimal_point, loc);
          kind = Kind::Error;
        }
      }
      if (state.safe() and (state.peek() == 'e' or state.peek() == 'E')) {
        auto loc = state.current_location();
        state.advance();
        if (state.safe() and (state.peek() == '+' or state.peek() == '-')) {
          state.advance();
        }
        if (not _consume_seq<error_t::expected_dec_literal, separator>(
              state, isdec, kind)) {
          state.report(error_t::missing_fp_exp, loc);
          kind = Kind::Error;
        }
      }
      if (kind != Kind::Error) {
        kind = Kind::Floating;
      }
    }
    return state.make_token(kind);
  }

  inline token::Token consume_number(State &state) {
    return consume_number_helper<'_'>(state);
  }
} // namespace rattle::lexer

