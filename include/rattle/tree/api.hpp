#pragma once

#include "forward.h"

namespace rattle::tree::visitor {
  // Convenience preprocessor macro
#define create_visitor(Type) virtual void visit(Type &) = 0
  // Expression visitor
  struct Expression {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(Type, _) create_visitor(expr::Type);
#include "require_pp/expression.h"
#include "require_pp/undefine.h"
    virtual ~Expression() = default;
  };

  // Statement visitor
  struct Statement {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(Type, _) create_visitor(stmt::Type);
#include "require_pp/statement.h"
#include "require_pp/undefine.h"
    virtual ~Statement() = default;
  };
#undef create_visitor
} // namespace rattle::tree::visitor

