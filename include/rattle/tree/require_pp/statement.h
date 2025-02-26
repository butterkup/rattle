
#include <rattle/token/require_pp/bits/keywords/statement.h>
#include <rattle/token/require_pp/bits/keywords/declare.h>

rattle_pp_token_macro(Block, "{ ... }") // Group of statements
rattle_pp_token_macro(ExprStmt, "expr") // print("Hello");  # the call expr here is an expression as a statement
rattle_pp_token_macro(Assignment, "=") // += -= /= *= =

