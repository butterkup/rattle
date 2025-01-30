#include "lexer.hpp"
#include <rattle/lexer.hpp>

namespace rattle::lexer {
  static void consume_string_escape(State &state, Token::Kind &kind) {
    using Kind = Token::Kind;
    if (not state.safe(2)) {
      kind = Kind::Error;
      state.advance();
      return state.report(error_t::unterminated_escape_in_string);
    }
    // clang-format off
    switch (state.peek(1)) {
      case '\\': state.advance(); break;
      case '\n': state.advance(); break;
      case 'n':  state.advance(); break;
      case 't':  state.advance(); break;
      case 'r':  state.advance(); break;
      case 'f':  state.advance(); break;
      case 'v':  state.advance(); break;
      case '\'':
      case '"':  state.advance(); break;
      case 'X':
      case 'x':
        if (state.safe(4)) {
          if (not(ishex(state.peek(2)) and ishex(state.peek(3)))) {
            kind = Kind::Error;
            auto loc = state.current_location();
            state.advance(); state.advance();
            if(ishex(state.peek())) state.advance();
            state.report(error_t::invalid_escape_hex_sequence, loc);
          }
        } else {
          kind = Kind::Error;
          auto loc = state.current_location();
          state.advance(); state.advance();
          if(state.safe() and ishex(state.peek())) state.advance();
          state.report(error_t::incomplete_escape_hex_sequence, loc);
        }                                                                break;
      default: {
        auto loc = state.current_location();
        state.advance(); state.advance();
        state.report(error_t::unrecognized_escape_character, loc);
      }
    }
    // clang-format on
  }

  template <bool multiline> static Token consume_string_helper(State &state) {
    auto kind = Token::Kind::String;
    char quote = state.advance();
    bool terminated = false;
    while (not state.empty()) {
      if constexpr (multiline) {
        if (state.safe(3) and state.peek() == quote and
            state.peek(1) == quote and state.peek(2) == quote) {
          state.advance();
          state.advance();
          state.advance();
          terminated = true;
        }
      } else {
        if (state.peek() == quote) {
          state.advance();
          terminated = true;
          break;
        }
        if (state.peek() == '\n') {
          return state.make_token(error_t::unterminated_single_line_string);
        }
      }
      if (state.peek() == '\\') {
        consume_string_escape(state, kind);
      } else {
        state.advance();
      }
    }
    if constexpr (multiline) {
      if (not terminated) {
        return state.make_token(error_t::unterminated_multi_line_string);
      }
    } else {
      if (not terminated) {
        return state.make_token(error_t::unterminated_single_line_string);
      }
    }
    return state.make_token(kind);
  }

  Token consume_multi_string(State &state) {
    return consume_string_helper<true>(state);
  }

  Token consume_single_string(State &state) {
    return consume_string_helper<false>(state);
  }
} // namespace rattle::lexer

