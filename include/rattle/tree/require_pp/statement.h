#include "ensure_def.h"

// clang-format off

// For instance, a block is composed of multiple statements requiring
// a container to store all of them, but in a pipelined system, we cannot
// chunks of arbitrary size going through, instead, the block is broken
// into three parts, the entry event, exit event and the statements in between
// these two constitute the children of the block, this way, the pipeline
// has small objects flowing through beautifully in harmony and no need for
// contaniners, just use events, further more, you can add custom events
// for the parser to comunicate with its owner about the state of things,
// expectations and what not.
rattle_pp_token_macro(Event, "Something happened") // Block entry/exit events
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
 *  if statement `if expr { something(); }`
 *  else statement (null expression) `else { something(); }`
*/
rattle_pp_token_macro(TkExprStmt, "tk expr body")

  // clang-format on

