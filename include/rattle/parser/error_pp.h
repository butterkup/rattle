#ifndef rattle_pp_error_macro
# error "must define macro `rattle_pp_error_macro(arg1)`"
# define rattle_pp_error_macro(e)
#endif

/* Scope symbol related errors */
rattle_pp_error_macro(dangling_brace)
rattle_pp_error_macro(dangling_paren)
rattle_pp_error_macro(dangling_bracket)
rattle_pp_error_macro(unterminated_brace)
rattle_pp_error_macro(unterminated_paren)
rattle_pp_error_macro(unterminated_bracket)

/* Other errors; generic. */
rattle_pp_error_macro(unexpected_token)
rattle_pp_error_macro(unterminated_statement)
rattle_pp_error_macro(unterminated_if_else_expr)
rattle_pp_error_macro(patial_notin_operator)

#undef rattle_pp_error_macro

