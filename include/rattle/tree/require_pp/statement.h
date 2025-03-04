#include "ensure_def.h"

// clang-format off

rattle_pp_token_macro(Block, "{ ... }") // Group of statements
rattle_pp_token_macro(If, "if cond { ... } else { ... }") // Conditional statement if-else
// print("Hello");  # the call expr here is an expression as a statement
rattle_pp_token_macro(ExprStmt, "expr")
// Behaves like a binary expression only that it is a statement unlike C/C++
rattle_pp_token_macro(Assignment, "=") // += -= /= *= =
/*
 * Capture single token statements and token expr statements. Examples:
 * continue
 * break
 * global a, b, c
 * nonlocal e, f, g
*/
rattle_pp_token_macro(TkExpr, "tk expr")
/*
 * Captures all statements of the form tk expr body. Examples:
 *  function declaration `def name(arg) { print(1); }`
 *  for loop statement `for i in seq { print(i); }`
 *  class declaration `class name(bases) { class_var = 90; }`
 *  while loop statement `while expr { something(); }`
*/
rattle_pp_token_macro(TkExprBlock, "tk expr body")

  // clang-format on

