#include <rattle/token/require_pp/bits/ensure_def.h>

rattle_pp_token_macro(BinaryExpr, "arg1 op arg2")
rattle_pp_token_macro(UnaryExpr, "op arg2")
rattle_pp_token_macro(IfElse, "expr1 if cond else expr2")
rattle_pp_token_macro(Group, "(expr)")
rattle_pp_token_macro(Primary, "literal")
rattle_pp_token_macro(Lambda, "|a, b| a * b + 5") // Anonymous functions
rattle_pp_token_macro(Call, "a(b)")
rattle_pp_token_macro(Subscript, "a[b]")
rattle_pp_token_macro(IsNot, "this is not that")
rattle_pp_token_macro(NotIn, "item not in seq")
rattle_pp_token_macro(In, "item in seq")
rattle_pp_token_macro(Dot, "expr.member")

