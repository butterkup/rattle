#include <optional>
#include <rattle/lexer/lexer.hpp>
#include <rattle/utility.hpp>

namespace rattle::lexer::internal {
  // Number lexer
  template <char separator> class Number {
    int flags;
    Cursor &cursor;
    // Set number kind
    void set_kind(token::kinds::Number::variants kind) {
      // Set the new kind but also don't overwrite the error bit
      // Remember it if it is on
      flags = kind | (flags & token::kinds::Number::Error);
    }
    // Error reporting from this class go through this method to ensure
    // the Error flag in the number variant is set.
    void report(error::Kind error, std::optional<State> mark = std::nullopt) {
      flags |= token::kinds::Number::Error;
      if (mark.has_value()) {
        cursor.report(error, mark.value());
      } else {
        cursor.report(error);
      }
    }

    // consumes a sequence digits as stated by the predicate
    // also picking up separators but not counting them as digits
    // in charge of reporting adjacent separators or trailing ones
    template <class Predicate>
    std::size_t eat_number_sequence(Predicate &&predicate) {
      std::size_t consumed{0};
      auto mark = cursor.bookmark();
      bool ensure_follows = false;
      auto is_sep = [](char ch) { return ch == separator; };
      std::size_t found = cursor.eat_while(is_sep);
      if (found > 0) {
        ensure_follows = true;
        if (found > 1) {
          report(error::Kind::repeated_numeric_separator, mark);
        }
      }
      for (;;) {
        found = cursor.eat_while(predicate);
        if (found == 0 or cursor.empty()) {
          if (ensure_follows) {
            report(error::Kind::trailing_numeric_separator, mark);
          }
          break;
        }
        ensure_follows = false;
        consumed += found;
        mark = cursor.bookmark();
        found = cursor.eat_while(is_sep);
        if (found > 0) {
          ensure_follows = true;
          if (found > 1) {
            report(error::Kind::repeated_numeric_separator, mark);
          }
        }
      }
      return consumed;
    }

    // how do we know we've consumed a whole sequence of digits as per the
    // predicate? well, any character that is not alphabetic is a valid end
    // of sequence marker, then we should consume alphanumeric as part and
    // report their consumption as errors affecting the whole number
    // as malformed
    // TODO: Naming leaves a lot to be desired
    template <error::Kind invalid, class Predicate>
    std::size_t eat_sequence_to_end(Predicate &&predicate) {
      std::size_t consumed = eat_number_sequence(predicate);
      auto mark = cursor.bookmark();
      if (cursor.eat_while(utility::is_identifier_body_char) != 0) {
        report(invalid, mark);
      }
      return consumed;
    }

    // helper function to ensure that we consume at least one digit
    // else report empty sequence, remember, separators are not counted
    // TODO: Better name please!
    template <error::Kind empty, error::Kind invalid, class Predicate>
    std::size_t eat_non_empty_sequence(Predicate &&predicate) {
      std::size_t const eaten = eat_sequence_to_end<invalid>(predicate);
      if (eaten == 0) {
        report(empty);
      }
      return eaten;
    }

    // helper function to consume based numbers
    // first consume the base marker, consume non empty
    // sequence of digits of that base then make and return
    // a token as per set value of kind
    template <token::kinds::Number::variants kind, error::Kind empty,
      error::Kind invalid, class Predicate>
    token::Token make_number_token(Predicate &&predicate) {
      set_kind(kind);
      cursor.eat(); // Consume the base specifier: x b o
      eat_non_empty_sequence<empty, invalid>(predicate);
      return cursor.make_token(token::kinds::Token::Number, flags);
    }

    // actual number lexer
    token::Token lex_number() {
      // Assume base 10, then diverge to the others
      // Also the error bit is set to 0 here to be turned
      // on when an error is reported
      flags = token::kinds::Number::Decimal;
      char const first = cursor.peek();
      cursor.eat();
      if (cursor.empty()) {
        return cursor.make_token(token::kinds::Number::Decimal);
      }
      if (first == '0') {
        switch (cursor.peek()) {
        case 'b':
        case 'B':
          return make_number_token<token::kinds::Number::Binary,
            error::Kind::empty_bin_literal, error::Kind::invalid_bin_character>(
            utility::is_binary);
        case 'o':
        case 'O':
          return make_number_token<token::kinds::Number::Octal,
            error::Kind::empty_oct_literal, error::Kind::invalid_oct_character>(
            utility::is_octal);
        case 'x':
        case 'X':
          return make_number_token<token::kinds::Number::Hexadecimal,
            error::Kind::empty_hex_literal, error::Kind::invalid_hex_character>(
            utility::is_hexadecimal);
        default:
          if (eat_number_sequence(utility::is_decimal) > 0) {
            report(error::Kind::leading_zero_in_decimal);
          }
        }
      }
      eat_number_sequence(utility::is_decimal);
      if (cursor.match('.')) {
        set_kind(token::kinds::Number::Float);
        if (eat_number_sequence(utility::is_decimal) == 0) {
          report(error::Kind::dangling_decimal_point);
        }
      }
      if (cursor.match('e') or cursor.match('E')) {
        set_kind(token::kinds::Number::Float);
        cursor.match('+') or cursor.match('-');
        if (eat_number_sequence(utility::is_decimal) == 0) {
          report(error::Kind::missing_exponent);
        }
      }
      eat_sequence_to_end<error::Kind::invalid_dec_character>(
        utility::is_decimal);
      return cursor.make_token(token::kinds::Token::Number, flags);
    }

  public:
    Number(Cursor &cursor): flags{}, cursor{cursor} {}
    token::Token lex() noexcept { return lex_number(); }
  };
  // A facade; hide all number lexing from the lexer that needn't know.
  token::Token Lexer::consume_number() noexcept {
    // delegate to the actual consumer passing a separator to it
    return Number<'_'>(cursor).lex();
  }
} // namespace rattle::lexer::internal

