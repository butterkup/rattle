#include "../ensure_def.h"

// clang-format off
rattle_pp_token_macro(Add, "a+b")
rattle_pp_token_macro(Subtract, "a-b")
rattle_pp_token_macro(Divide, "a/b")
rattle_pp_token_macro(Multiply, "a*b")
rattle_pp_token_macro(Subscript, "a[b]")
rattle_pp_token_macro(Call, "a(b)")
rattle_pp_token_macro(Dot, "a.b")
rattle_pp_token_macro(Is, "expr1 is expr2")
rattle_pp_token_macro(IsNot, "expr1 is not expr2")
rattle_pp_token_macro(In, "expr in seq")
rattle_pp_token_macro(NotIn, "expr not in seq")

