#include "rattle/lexer/error.hpp"
#include "rattle/rattle.hpp"
#include "rattle/token/token.hpp"
#include <optional>
#include <rattle/lexer/lexer.hpp>

namespace rattle::lexer::internal {
  // Custom filter, lambda function getting too big. Hopefully compiler inlines
  template <bool ismultiline, bool israw> struct Filter {
    const char quote;
    constexpr Filter(char quote) noexcept: quote(quote) {}
    inline bool operator()(char ch) const noexcept { return invoke(ch); }
    inline bool invoke(char ch) const noexcept;
  };
  // Overloads
  template <> inline bool Filter<true, true>::invoke(char ch) const noexcept {
    return ch != quote;
  }
  template <> inline bool Filter<true, false>::invoke(char ch) const noexcept {
    return ch != quote and ch != '\\';
  }
  template <> inline bool Filter<false, true>::invoke(char ch) const noexcept {
    return ch != quote and ch != '\n';
  }
  template <> inline bool Filter<false, false>::invoke(char ch) const noexcept {
    return ch != quote and ch != '\n' and ch != '\\';
  }

  // String lexer
  class String {
    int flags;
    Cursor &cursor;

    // All errors are reported in one place to ensure error bit is activated
    void report(error::Kind kind, std::optional<State> mark = std::nullopt) {
      flags |= token::kinds::String::Error;
      if (mark.has_value()) {
        cursor.report(kind, mark.value());
      } else {
        cursor.report(kind);
      }
    }

    // Handle in string escapes.
    void escape_sequence() noexcept {
      auto mark = cursor.bookmark();
      cursor.eat(); // Consume the escaper
      if (cursor.empty()) {
        report(error::Kind::partial_string_escape, mark);
      } else {
        switch (cursor.peek()) {
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
          cursor.eat();
          break;
        case 'x':
        case 'X': // Hexadecimal escape: \xab \x23 \x10
          if (cursor.safe(2)) {
            if (not(utility::is_hexadecimal(cursor.peek(1)) and
                    utility::is_hexadecimal(cursor.peek(2)))) {
              report(error::Kind::invalid_escape_hex_sequence, mark);
            } else {
              cursor.eat();
              cursor.eat();
            }
          } else {
            report(error::Kind::partial_string_hex_escape, mark);
          }
          cursor.eat();
          break;
        default:
          cursor.eat();
          report(error::Kind::invalid_escape_sequence, mark);
        }
      }
    }

    // Generic function to lex strings. Can handle multiline and single
    // line string lexing and the compiler can evaluate some portions
    // (expressions with multiline constant) of the function removing
    // dead code and optimize for each listed variant.
    template <bool ismultiline, bool israw>
    void consume_string_variant() noexcept {
      const char quote = cursor.eat();
      if constexpr (israw) {
        flags |= token::kinds::String::Raw;
      }
      if constexpr (ismultiline) {
        flags |= token::kinds::String::Multiline;
        cursor.eat();
        cursor.eat();
      }

      for (;;) {
        cursor.eat_while(Filter<ismultiline, israw>(quote));
        if (cursor.empty()) {
          report(ismultiline ? error::Kind::unterminated_multi_line_string :
                               error::Kind::unterminated_single_line_string);
          return;
        } else {
          switch (cursor.peek()) {
          case '\\':
            if constexpr (israw) {
              // raw strings escape anything
              auto mark = cursor.bookmark();
              cursor.eat(); // Consume escaper
              if (cursor.safe()) {
                cursor.eat(); // Whatever the escaped is, consume it!
              } else {
                report(error::Kind::partial_string_escape, mark);
              }
            } else {
              escape_sequence();
            }
            break;
          case '\n':
            if constexpr (ismultiline) {
              // multiline strings ignore newlines
              unreachable();
            } else {
              report(error::Kind::unterminated_single_line_string);
              return;
            }
            break;
          default:
            if constexpr (ismultiline) {
              if (cursor.match(quote) and cursor.match(quote) and
                  cursor.match(quote)) {
                return;
              }
            } else {
              if (cursor.match(quote)) {
                return;
              }
            }
          }
        }
      }
    }

    // Determine which string variant to call and delegate lexing to it
    // forming a token on the way out
    template <bool israw> token::Token lex_string() noexcept {
      flags = token::kinds::String::Normal;
      const char quote = cursor.peek();
      if (cursor.safe(2) and cursor.peek(1) == quote and
          cursor.peek(2) == quote) {
        consume_string_variant<true, israw>();
      } else {
        consume_string_variant<false, israw>();
      }
      return cursor.make_token(token::kinds::Token::String, flags);
    }

  public:
    String(Cursor &cursor): flags{0}, cursor{cursor} {}
    // Cannot accept flags from outside since it would invalidate the lexer, but
    // some flags are pretty necessary for checking for certain events like if
    // syntactically the string should be lexed as a raw string or multiline as seen
    // here and in the lex_string functions
    token::Token lex(int flags) {
      if (flags & token::kinds::String::Raw) {
        return lex_string<true>();
      } else {
        return lex_string<false>();
      }
    }
  };

  // A facade, hides all string lexing weirdness from the lexer
  token::Token Lexer::consume_string(int flags) noexcept {
    // Call the actual lexer giving it lexer provided flags
    return String(cursor).lex(flags);
  }
} // namespace rattle::lexer::internal

