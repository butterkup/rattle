#include "ensure_def.h"

// clang-format off

// Represent unary expressions
rattle_pp_token_macro(UnaryExpr, "op arg1")
// Represent binary expressions
rattle_pp_token_macro(BinaryExpr, "arg1 op arg2")
// Represent literal values
rattle_pp_token_macro(Literal, "literal")
/* Capture all expressions representable with 2 expressions and 2 tokens. Examples:
  * call expression `expr1(expr2)`
  * subscript expression `expr1[expr2]`
  * is not identity check `expr1 is not expr2`
  * not in member check `expr1 not in expr2`
  * lambda function `|expr1| expr2`
  * if-else `expr11 if expr12 else expr2` in which expr1 is the binary expression `expr1 if expr2`
*/
rattle_pp_token_macro(BiExprBiTk, "tk1 tk2 expr1 expr2")

// clang-format on

