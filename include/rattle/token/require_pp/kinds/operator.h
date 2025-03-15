#include "../ensure_def.h"

// NOTE: Some keywords like `and`, `or` and `not` are operators too
// but including them here means they exist in two places, as keywords
// and operators, therefore, this file only deals in symbolic operators

// Comparison operators
rattle_pp_token_macro(EqualEqual, "==")
rattle_pp_token_macro(NotEqual, "!=")
rattle_pp_token_macro(LessEqual, "<=")
rattle_pp_token_macro(LessThan, "<")
rattle_pp_token_macro(GreaterEqual, ">=")
rattle_pp_token_macro(GreaterThan, ">")

// Other operators
rattle_pp_token_macro(Plus, "+")
rattle_pp_token_macro(Minus, "-")
rattle_pp_token_macro(Slash, "/")
rattle_pp_token_macro(Star, "*")
rattle_pp_token_macro(Dot, ".")

// Doesn't exist at runtime, simply helps hold lists of expressions
rattle_pp_token_macro(Comma, ",")
