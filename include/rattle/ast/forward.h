#pragma once

namespace rattle::ast {
  struct Node;
  struct Stmt;
  struct Expr;

#define rattle_pp_token_macro(Name, _) struct Name;

  namespace stmt {
#include "require_pp/statement.h"
  }

  namespace expr {
#include "require_pp/expression.h"
  }

#undef rattle_pp_token_macro

  namespace visitor {
#define CREATE_VISIT_METHOD(Name) virtual void visit(Name &) noexcept = 0;

    struct StmtVisitor {
#define rattle_pp_token_macro(Name, _) CREATE_VISIT_METHOD(stmt::Name)
#include "require_pp/statement.h"
#undef rattle_pp_token_macro
    };

    struct ExprVisitor {
#define rattle_pp_token_macro(Name, _) CREATE_VISIT_METHOD(expr::Name)
#include "require_pp/expression.h"
#undef rattle_pp_token_macro
    };

#undef CREATE_VISIT_METHOD
  } // namespace visitor
} // namespace rattle::ast

