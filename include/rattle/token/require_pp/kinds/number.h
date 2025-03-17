#include "../ensure_def.h"

// The numbers in the second argument tell what kind of
// number the token holds except for the Error which is
// a flag that tells if an error was encountered while
// lexing the number.
// If off, the number is well formed
rattle_pp_token_macro(Error, 1 << 16) // The error bit is far ahead
// The lower bits capture the number variant
rattle_pp_token_macro(Float, 0) // Base 10 floating point number
rattle_pp_token_macro(Binary, 1) // Base 2 number
rattle_pp_token_macro(Octal, 2) // Base 8 number
rattle_pp_token_macro(Decimal, 3) // Base 10 number
rattle_pp_token_macro(Hexadecimal, 4) // Base 16 number
