#pragma once

#include "forward.h"

namespace rattle::tree::visitor {
  // Convenience preprocessor macro
#define create_visitor(Type) virtual void visit(Type &) = 0
  // Expression visitor
  struct ExprVisitor {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(Type, _) create_visitor(expr::Type);
#include "require_pp/expression.h"
#include "require_pp/undefine.h"
    virtual ~ExprVisitor() = default;
  };

  // Statement visitor
  struct StmtVisitor {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(Type, _) create_visitor(stmt::Type);
#include "require_pp/statement.h"
#include "require_pp/undefine.h"
    virtual ~StmtVisitor() = default;
  };
#undef create_visitor
} // namespace rattle::tree::visitor

