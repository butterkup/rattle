#pragma once

#include <cstddef>

namespace rattle {
  enum class error_t;
  struct Location {
    std::size_t line, column, offset;
  };
  struct Error {
    error_t description;
    Location start, end;
  };
  enum class error_t {
    invalid_character,
    globl_escapee_missing,
    globl_invalid_escapee,
    malformed_not_equal,
    unterminated_single_string,
    unterminated_multi_string,
    missing_literal_escapee,
    too_few_literal_escapees,
    invalid_hex_escapees,
    invalid_literal_escapee,
    multi_adj_num_separators,
    dangling_num_separator,
    empty_hex_literal,
    empty_oct_literal,
    empty_dec_literal,
    empty_bin_literal,
    expected_hex_literal,
    expected_oct_literal,
    expected_dec_literal,
    expected_bin_literal,
    zero_prefixed_dec,
    dangling_decimal_point,
    missing_fp_exp,
  };
} // namespace rattle

