#include "ensure_def.h"

rattle_pp_token_macro(Identifier, Variable) // All names, keywords and variables.
rattle_pp_token_macro(String, *) // String literal values.; raw, multiline, singline, ...
rattle_pp_token_macro(Number, *) // Number literal values; octal, binary, hexadecimal, float, ...
