#include "ensure_def.h"

#ifndef rattle_numbers_h
# define rattle_numbers_h

TOKEN_MACRO(Hexadecimal, "") // 0[xX][0-9A-Fa-f]+
TOKEN_MACRO(Float, "")       // [0-9]+.[0-9]+([eE][+-]?[0-9]+)?
TOKEN_MACRO(Decimal, "")     // [1-9][0-9]*
TOKEN_MACRO(Binary, "")      // 0[bB][01]+
TOKEN_MACRO(Octal, "")       // 0[oO][0-7]+

#endif
