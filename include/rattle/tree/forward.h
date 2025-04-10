#pragma once

namespace rattle::tree {
  // Base class for all nodes
  struct Node;
  // Base class for all expressions
  struct Expr;
  // Base class for all statements
  struct Stmt;

  namespace stmt {
    // Forward declare statement nodes
#define rattle_pp_token_macro(kind, _) struct kind;
#include "require_pp/statement.h"
  } // namespace stmt
  namespace expr {
    // Forward declare expression nodes
#include "require_pp/expression.h"
#undef rattle_pp_token_macro
  } // namespace expr

  namespace visitor {
    // Forward declare visitor base classes and expr kind resolver
    struct Statement;
    struct Expression;
  } // namespace visitor
} // namespace rattle::tree

