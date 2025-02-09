#include "utility.hpp"
#include <cctype>
#include <lexer/lexer.hpp>

namespace rattle::lexer::internal {
  namespace numbers {
    // consumes a sequence digits as stated by the predicate
    // also picking up separators but not counting them as digits
    // in charge of reporting adjacent separators or trailing ones
    template <char sep, class predicate_t>
    static std::size_t eat_number_sequence(
      cursor_t &base, token_kind_t &kind, predicate_t &&predicate) {
      std::size_t consumed{0};
      auto mark = base.bookmark();
      bool ensure_follows = false;
      auto is_sep = [](char ch) { return ch == sep; };
      std::size_t found = base.eat_while(is_sep);
      if (found > 0) {
        ensure_follows = true;
        if (found > 1) {
          base.report(error_kind_t::repeated_numeric_separator, mark);
          kind = token_kind_t::Error;
        }
      }
      for (;;) {
        found = base.eat_while(predicate);
        if (found == 0 or base.empty()) {
          if (ensure_follows) {
            base.report(error_kind_t::trailing_numeric_separator, mark);
            kind = token_kind_t::Error;
          }
          break;
        }
        ensure_follows = false;
        consumed += found;
        mark = base.bookmark();
        found = base.eat_while(is_sep);
        if (found > 0) {
          ensure_follows = true;
          if (found > 1) {
            base.report(error_kind_t::repeated_numeric_separator, mark);
            kind = token_kind_t::Error;
          }
        }
      }
      return consumed;
    }
    // how do we know we've consumed a whole sequence of digits as per the
    // predicate? well, any character that is not alphabetic is a valid end
    // of sequence marker, then we should consume slphabetic as part and
    // report their consumption as errors affecting the whole number
    // as malformed
    // TODO: Naming leaves a lot to be desired
    template <char sep, error_kind_t invalid, class predicate_t>
    static std::size_t eat_sequence_to_end(
      cursor_t &base, token_kind_t &kind, predicate_t &&predicate) {
      std::size_t consumed = eat_number_sequence<sep>(base, kind, predicate);
      auto mark = base.bookmark();
      if (base.eat_while(std::isalpha) != 0) {
        base.report(invalid, mark);
      }
      return consumed;
    }
    // helper function to ensure that we consume at least one digit
    // else report empty sequence, remember, separators are not counted
    // TODO: Better name please!
    template <char sep, error_kind_t empty, error_kind_t invalid,
      class predicate_t>
    static std::size_t eat_non_empty_sequence(
      cursor_t &base, token_kind_t &kind, predicate_t &&predicate) {
      std::size_t const eaten =
        eat_sequence_to_end<sep, invalid>(base, kind, predicate);
      if (eaten == 0) {
        base.report(empty);
      }
      return eaten;
    }
    // helper function to consume based numbers
    // first consume the base marker, consume non empty
    // sequence of digits of that base then make and return
    // a token as per set value of kind
    template <char sep, error_kind_t empty, error_kind_t invalid,
      class predicate_t>
    static token_t make_number_token(
      cursor_t &base, token_kind_t kind, predicate_t &&predicate) {
      base.eat();
      eat_non_empty_sequence<sep, empty, invalid>(base, kind, predicate);
      return base.make_token(kind);
    }
    // actual number lexer
    template <char sep> token_t consume_number(cursor_t &base) {
      char const first = base.peek();
      base.eat();
      token_kind_t kind = token_kind_t::Decimal;
      if (base.empty()) {
        return base.make_token(kind);
      }
      if (first == '0') {
        switch (base.peek()) {
        case 'b':
        case 'B':
          return make_number_token<sep, error_kind_t::empty_bin_literal,
            error_kind_t::invalid_bin_character>(
            base, token_kind_t::Binary, utility::is_binary);
        case 'o':
        case 'O':
          return make_number_token<sep, error_kind_t::empty_oct_literal,
            error_kind_t::invalid_oct_character>(
            base, token_kind_t::Octal, utility::is_octal);
        case 'x':
        case 'X':
          return make_number_token<sep, error_kind_t::empty_hex_literal,
            error_kind_t::invalid_hex_character>(
            base, token_kind_t::Hexadecimal, utility::is_hexadecimal);
        default:
          if (eat_number_sequence<sep>(base, kind, utility::is_decimal) > 0) {
            base.report(error_kind_t::leading_zero_in_decimal);
          }
        }
      }
      eat_number_sequence<sep>(base, kind, utility::is_decimal);
      if (base.match('.')) {
        kind = token_kind_t::Float;
        if (eat_number_sequence<sep>(base, kind, utility::is_decimal) == 0) {
          base.report(error_kind_t::dangling_decimal_point);
        }
      }
      if (base.match('e') or base.match('E')) {
        kind = token_kind_t::Float;
        base.match('+') or base.match('-');
        if (eat_number_sequence<sep>(base, kind, utility::is_decimal) == 0) {
          base.report(error_kind_t::missing_exponent);
        }
      }
      eat_sequence_to_end<sep, error_kind_t::invalid_dec_character>(
        base, kind, utility::is_decimal);
      return base.make_token(kind);
    }
  } // namespace numbers

  token_t lexer_t::consume_number() {
    // delegate to the actual consumer passing a separator to it
    return numbers::consume_number<'_'>(base);
  }
} // namespace rattle::lexer::internal

