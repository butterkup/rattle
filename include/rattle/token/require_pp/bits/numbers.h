#include "ensure_def.h"

#ifndef rattle_numbers_h
# define rattle_numbers_h

rattle_pp_token_macro(Hexadecimal, "") // 0[xX][0-9A-Fa-f]+
rattle_pp_token_macro(Float, "")       // [0-9]+.[0-9]+([eE][+-]?[0-9]+)?
rattle_pp_token_macro(Decimal, "")     // [1-9][0-9]*
rattle_pp_token_macro(Binary, "")      // 0[bB][01]+
rattle_pp_token_macro(Octal, "")       // 0[oO][0-7]+

#endif
