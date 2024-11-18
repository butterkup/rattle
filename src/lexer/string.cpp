#include "lexer.hpp"
#include <rattle/lexer.hpp>

namespace rattle::lexer {
  static char hex2ch(char hex) {
    return hex - (hex >= 'a' ? 87 : (hex >= 'A' ? 55 : 48));
  }
  static void consume_string_escape(State &state, Token::Kind &kind) {
    using Kind = Token::Kind;
    if (not state.safe(2)) {
      kind = Kind::Error;
      state.advance();
      return state.report(error_t::unterminated_escape_in_string);
    }
    // clang-format off
    switch (state.peek(1)) {
      case '\\':state.advance(); state.advance_erase();                  break;
      case '\n':state.advance_erase(); state.advance_erase();            break;
      case 'n': state.advance_erase(); state.advance_replace('\n');      break;
      case 't': state.advance_erase(); state.advance_replace('\t');      break;
      case 'r': state.advance_erase(); state.advance_replace('\r');      break;
      case 'f': state.advance_erase(); state.advance_replace('\f');      break;
      case 'v': state.advance_erase(); state.advance_replace('\v');      break;
      case '\'':
      case '"': state.advance_erase(); state.advance();                  break;
      case 'X':
      case 'x':
        if (state.safe(4)) {
          if (ishex(state.peek(2)) and ishex(state.peek(3))) {
            char value = (hex2ch(state.peek(2)) << 4) | hex2ch(state.peek(3));
            state.advance_erase(); state.advance_erase();
            state.advance_erase();
            state.advance_replace(value);
          } else {
            kind = Kind::Error;
            auto loc = state.current_location();
            state.advance(); state.advance();
            state.report(error_t::invalid_escape_hex_sequence, loc);
          }
        } else {
          kind = Kind::Error;
          auto loc = state.current_location();
          state.advance(); state.advance();
          if(state.safe()) state.advance();
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
      if (state.peek() == quote) {
        state.advance();
        terminated = true;
        break;
      } else if (not multiline and state.peek() == '\n') {
        return state.make_token(error_t::unterminated_single_line_string);
      } else if (state.peek() == '\\') {
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

