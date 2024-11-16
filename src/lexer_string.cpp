#include "lexer.hpp"
#include <rattle/lexer.hpp>

namespace rattle::lexer {
  namespace __detail {
    char _hex2char(char hex) {
      return hex - (isdecimal(hex) ? '0' : (std::islower(hex) ? 'a' : 'A'));
    }
    void consume_string_escape(State &state, token::Token::Kind &kind) {
      using token::Token::Kind::Error;
      if (not state.safe(2)) {
        kind = Error;
        return state.report(error_t::missing_literal_escapee);
      }
      // clang-format off
      switch (state.peek(1)) {
        case '\\':state.advance(); state.erase_char(); break;
        case '\n':state.erase_char(); state.advance(); break;
        case 'n': state.erase_char(); state.advance(); state.replace_char('\n'); break;
        case 'r': state.erase_char(); state.advance(); state.replace_char('\r'); break;
        case 'f': state.erase_char(); state.advance(); state.replace_char('\f'); break;
        case 'v': state.erase_char(); state.advance(); state.replace_char('\v'); break;
        case '\'':
        case '"': state.erase_char(); state.advance(); break;
        case 'X':
        case 'x':
          if (state.safe(4)) {
            if (ishexadecimal(state.peek(2)) and ishexadecimal(state.peek(3))) {
              char value = (_hex2char(state.peek(2)) << 4) | _hex2char(state.peek(3));
              state.erase_char();
              state.erase_char();
              state.erase_char();
              state.advance();
              state.replace_char(value);
            } else {
              kind = Error;
              auto loc = state.current_location();
              state.advance();
              state.advance();
              state.report(error_t::invalid_hex_escapees, loc);
            }
          } else {
            kind = Error;
            auto loc = state.current_location();
            state.advance();
            state.advance();
            state.report(error_t::too_few_literal_escapees, loc);
          } break;
        default: {
          auto loc = state.current_location();
          state.advance();
          state.report(error_t::invalid_literal_escapee, loc);
        }
      }
      // clang-format on
    }

    template <bool multiline> token::Token consume_string(State &state) {
      auto kind = token::Token::Kind::String;
      char quote = state.advance();
      bool terminated = false;
      while (not state.empty()) {
        if (state.peek() == quote) {
          state.advance();
          terminated = true;
          break;
        } else if constexpr (not multiline) {
          if (state.peek() == '\n') {
            return state.make_token(error_t::unterminated_single_string);
          }
        } else if (state.peek() == '\\') {
          consume_string_escape(state, kind);
        } else {
          state.advance();
        }
      }
      if constexpr (multiline) {
        if (not terminated) {
          return state.make_token(error_t::unterminated_multi_string);
        }
      } else {
        if (not terminated) {
          return state.make_token(error_t::unterminated_single_string);
        }
      }
      return state.make_token(kind);
    }
  } // namespace __detail
  inline token::Token consume_multi_string(State &state) {
    return __detail::consume_string<true>(state);
  }
  inline token::Token consume_single_string(State &state) {
    return __detail::consume_string<false>(state);
  }
} // namespace rattle::lexer

