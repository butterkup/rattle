#ifndef ERROR_MACRO
# error "macro ERROR_MACRO(arg1) must be defined"
# define ERROR_MACRO(e)
#endif

/* String related errors */
ERROR_MACRO(unterminated_single_line_string)
ERROR_MACRO(unterminated_multi_line_string)
ERROR_MACRO(partial_string_escape)
ERROR_MACRO(partial_string_hex_escape)
ERROR_MACRO(invalid_escape_sequence)
ERROR_MACRO(invalid_escape_hex_sequence)

/* Number related errors */
/* - separator errors */
ERROR_MACRO(repeated_numeric_separator)
ERROR_MACRO(trailing_numeric_separator)
/* - float errors */
ERROR_MACRO(dangling_decimal_point)
ERROR_MACRO(missing_exponent)
/* - invalid character errors */
ERROR_MACRO(leading_zero_in_decimal)
ERROR_MACRO(invalid_hex_character)
ERROR_MACRO(invalid_oct_character)
ERROR_MACRO(invalid_dec_character)
ERROR_MACRO(invalid_bin_character)
/* - empty literals */
ERROR_MACRO(empty_hex_literal)
ERROR_MACRO(empty_oct_literal)
ERROR_MACRO(empty_bin_literal)

/* Top level errors */
ERROR_MACRO(partially_formed_crlf)
ERROR_MACRO(partial_toplvl_escape)
ERROR_MACRO(invalid_toplvl_escape_sequence)
ERROR_MACRO(unrecognized_toplvl_character)
ERROR_MACRO(partial_not_equal)

#undef ERROR_MACRO
