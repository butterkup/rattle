#include <cassert>
#include <rattle/lexer/lexer.hpp>
#include <rattle/utility.hpp>

namespace rattle::lexer::internal {
  namespace strings {
    // Handle in string escapes.
    static void escape_sequence(Cursor &base, token::Kind &kind) noexcept {
      auto mark = base.bookmark();
      base.eat(); // Consume the escaper
      if (base.empty()) {
        kind = token::Kind::Error;
        base.report(error::Kind::partial_string_escape, mark);
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
              kind = token::Kind::Error;
              base.report(error::Kind::invalid_escape_hex_sequence, mark);
            } else {
              base.eat();
              base.eat();
            }
          } else {
            kind = token::Kind::Error;
            base.report(error::Kind::partial_string_hex_escape, mark);
          }
          base.eat();
          break;
        default:
          base.eat();
          base.report(error::Kind::invalid_escape_sequence, mark);
        }
      }
    }
    // Custom filter, lambda function getting too big. Hopefully compiler inlines
    template <bool ismultiline, bool israw> struct Filter {
      const char quote;
      constexpr Filter(char quote) noexcept: quote(quote) {}
      bool operator()(char ch) const noexcept { return invoke(ch); }
      bool invoke(char ch) const noexcept;
    };
    // Overloads
    template <> inline bool Filter<true, true>::invoke(char ch) const noexcept {
      return ch != quote;
    }
    template <>
    inline bool Filter<true, false>::invoke(char ch) const noexcept {
      return ch != quote and ch != '\\';
    }
    template <>
    inline bool Filter<false, true>::invoke(char ch) const noexcept {
      return ch != quote and ch != '\n';
    }
    template <>
    inline bool Filter<false, false>::invoke(char ch) const noexcept {
      return ch != quote and ch != '\n' and ch != '\\';
    }
    // Generic function to lex strings. Can handle multiline and single
    // line string lexing and the compiler can evaluate some portions
    // (expressions with multiline constant) of the function removing
    // dead code and optimize for each listed variant.
    template <bool ismultiline, bool israw>
    static token::Kind consume_string_variant(Cursor &base) noexcept {
      const char quote = base.eat();
      token::Kind kind = israw ? token::Kind::RawSingleLineString :
                                 token::Kind::SingleLineString;
      if constexpr (ismultiline) {
        kind = israw ? token::Kind::RawMultilineString :
                       token::Kind::MultilineString;
        base.eat();
        base.eat();
      }
      for (;;) {
        base.eat_while(Filter<ismultiline, israw>(quote));
        if (base.empty()) {
          kind = token::Kind::Error;
          base.report(ismultiline ?
                        error::Kind::unterminated_multi_line_string :
                        error::Kind::unterminated_single_line_string);
          return kind;
        } else {
          switch (base.peek()) {
          case '\\':
            if constexpr (israw) {
              // raw strings escape anything
              auto mark = base.bookmark();
              base.eat(); // Consume escaper
              if (base.safe()) {
                base.eat(); // Whatever the escaped is, consume it!
              } else {
                base.report(error::Kind::partial_string_escape, mark);
              }
            } else {
              escape_sequence(base, kind);
            }
            break;
          case '\n':
            if constexpr (ismultiline) {
              // multiline strings ignore newlines
              assert(false);
            } else {
              kind = token::Kind::Error;
              base.report(error::Kind::unterminated_single_line_string);
              return kind;
            }
            break;
          default:
            if constexpr (ismultiline) {
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
    template <bool raw>
    static token::Token consume_string(Cursor &base) noexcept {
      const char quote = base.peek();
      return base.make_token(
        base.safe(2) and base.peek(1) == quote and base.peek(2) == quote ?
          consume_string_variant<true, raw>(base) :
          consume_string_variant<false, raw>(base));
    }
  } // namespace strings

  // A Facade; hides all the string lexing complexity from Cursor
  token::Token Lexer::consume_string() noexcept {
    return strings::consume_string<false>(base);
  }
  token::Token Lexer::consume_raw_string() noexcept {
    return strings::consume_string<true>(base);
  }
} // namespace rattle::lexer::internal

