#include <rattle/token/require_pp/bits/ensure_def.h>

rattle_pp_token_macro(IfElse, "expr1 if cond else expr2")
rattle_pp_token_macro(Group, "(expr)")
rattle_pp_token_macro(Primary, "literal")
rattle_pp_token_macro(Lambda, "|a, b| a * b + 5") // Anonymous functions
rattle_pp_token_macro(Call, "a(b)")
rattle_pp_token_macro(Subscript, "a[b]")
rattle_pp_token_macro(IsNot, "is not")
rattle_pp_token_macro(NotIn, "not in")

