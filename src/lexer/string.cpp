#include "utility.hpp"
#include <cassert>
#include <rattle/lexer/lexer.hpp>

namespace rattle::lexer::internal {
  namespace strings {
    // Handle in string escapes.
    static void escape_sequence(cursor_t &base, token_kind_t &kind) {
      auto mark = base.bookmark();
      base.eat(); // Consume the escaper
      if (base.empty()) {
        kind = token_kind_t::Error;
        base.report(error_kind_t::partial_string_escape, mark);
      } else {
        switch (base.peek()) {
        case '0':  // Null
        case 'n':  // Newline
        case 'r':  // Carriage return
        case 'v':  // Vertical tab
        case 'f':  // Form feed
        case 't':  // Tab
        case 'b':  // Backspace
        case 'a':  // Alarm
        case '\'': // Single quote
        case '"':  // Double quote
        case '\\': // Backward slash
          base.eat();
          break;
        case 'x':
        case 'X': // Hexadecimal escape: \xab \x23 \x10
          if (base.safe(2)) {
            if (not(utility::is_hexadecimal(base.peek(1)) and
                    utility::is_hexadecimal(base.peek(2)))) {
              kind = token_kind_t::Error;
              base.report(error_kind_t::invalid_escape_hex_sequence, mark);
            }
            base.eat();
            base.eat();
          } else {
            kind = token_kind_t::Error;
            base.report(error_kind_t::partial_string_hex_escape, mark);
          }
          base.eat();
          break;
        default:
          base.eat();
          base.report(error_kind_t::invalid_escape_sequence);
        }
      }
    }
    // Generic function to lex strings. Can handle multiline and single
    // line string lexing and the compiler can evaluate some portions
    // (expressions with multiline constant) of the function removing
    // dead code and optimize for each listed variant.
    template <bool multiline>
    static token_kind_t consume_string_variant(cursor_t &base) {
      const char quote = base.eat();
      token_kind_t kind = token_kind_t::SingleLineString;
      if constexpr (multiline) {
        kind = token_kind_t::MultilineString;
        base.eat();
        base.eat();
      }
      for (;;) {
        base.eat_while([ quote ](char ch) {
          return multiline ? ch != quote and ch != '\\' :
                             ch != quote and ch != '\\' and ch != '\n';
        });
        if (base.empty()) {
          kind = token_kind_t::Error;
          base.report(multiline ?
                        error_kind_t::unterminated_multi_line_string :
                        error_kind_t::unterminated_single_line_string);
          return kind;
        } else {
          switch (base.peek()) {
          case '\\':
            escape_sequence(base, kind);
            break;
          case '\n':
            if constexpr (multiline) {
              // should never hit this.
              assert(false);
            } else {
              kind = token_kind_t::Error;
              base.report(error_kind_t::unterminated_single_line_string);
              return kind;
            }
            break;
          default:
            if constexpr (multiline) {
              if (base.match(quote) and base.match(quote) and
                  base.match(quote)) {
                return kind;
              }
            } else {
              if (base.match(quote)) {
                return kind;
              }
            }
          }
        }
      }
    }

    // Determine which string variant to call and delegate lexing to it
    // forming a token on the way out
    static token_t consume_string(cursor_t &base) {
      const char quote = base.peek();
      return base.make_token(
        base.safe(2) and base.peek(1) == quote and base.peek(2) == quote ?
          consume_string_variant<true>(base) :
          consume_string_variant<false>(base));
    }
  } // namespace strings

  // A Facade; hides all the string lexing complexity from cursor_t
  token_t lexer_t::consume_string() { return strings::consume_string(base); }
} // namespace rattle::lexer::internal

