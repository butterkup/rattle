#ifndef rattle_pp_error_macro
# error "must define macro `rattle_pp_error_macro(arg1)`"
# define rattle_pp_error_macro(e)
#endif

/* String related errors */
rattle_pp_error_macro(unterminated_single_line_string)
rattle_pp_error_macro(unterminated_multi_line_string)
rattle_pp_error_macro(partial_string_escape)
rattle_pp_error_macro(partial_string_hex_escape)
rattle_pp_error_macro(invalid_escape_sequence)
rattle_pp_error_macro(invalid_escape_hex_sequence)

/* Number related errors */
/* - separator errors */
rattle_pp_error_macro(repeated_numeric_separator)
rattle_pp_error_macro(trailing_numeric_separator)
/* - float errors */
rattle_pp_error_macro(dangling_decimal_point)
rattle_pp_error_macro(missing_exponent)
/* - invalid character errors */
rattle_pp_error_macro(leading_zero_in_decimal)
rattle_pp_error_macro(invalid_hex_character)
rattle_pp_error_macro(invalid_oct_character)
rattle_pp_error_macro(invalid_dec_character)
rattle_pp_error_macro(invalid_bin_character)
/* - empty literals */
rattle_pp_error_macro(empty_hex_literal)
rattle_pp_error_macro(empty_oct_literal)
rattle_pp_error_macro(empty_bin_literal)

/* Top level errors */
rattle_pp_error_macro(partially_formed_crlf)
rattle_pp_error_macro(partial_toplvl_escape)
rattle_pp_error_macro(invalid_toplvl_escape_sequence)
rattle_pp_error_macro(unrecognized_toplvl_character)
rattle_pp_error_macro(partial_not_equal)

#undef rattle_pp_error_macro
