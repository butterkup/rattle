#ifndef ERROR_MACRO
# error "macro ERROR_MACRO(arg1) must be defined"
# define ERROR_MACRO(e)
#endif

ERROR_MACRO(unrecognized_character)
ERROR_MACRO(unterminated_escape_sequence)
ERROR_MACRO(invalid_escape_sequence)
ERROR_MACRO(incomplete_not_equal_operator)
ERROR_MACRO(invalid_escape_hex_sequence)
ERROR_MACRO(incomplete_escape_hex_sequence)
ERROR_MACRO(unrecognized_escape_character)
ERROR_MACRO(unterminated_single_line_string)
ERROR_MACRO(unterminated_multi_line_string)
ERROR_MACRO(unterminated_escape_in_string)
ERROR_MACRO(repeated_numeric_separator)
ERROR_MACRO(trailing_numeric_separator)
ERROR_MACRO(dangling_decimal_point)
ERROR_MACRO(missing_exponent_after_e)
ERROR_MACRO(missing_fractinal_part)
ERROR_MACRO(leading_zero_in_decimal)
ERROR_MACRO(invalid_hex_character)
ERROR_MACRO(invalid_oct_character)
ERROR_MACRO(invalid_dec_character)
ERROR_MACRO(invalid_bin_character)
ERROR_MACRO(empty_hex_literal)
ERROR_MACRO(empty_oct_literal)
ERROR_MACRO(empty_bin_literal)

#undef ERROR_MACRO
