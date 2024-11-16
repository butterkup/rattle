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
  };
} // namespace rattle

