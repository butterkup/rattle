#pragma once

#include "forward.h"
#include "nodes_kinds.h"

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
#define rattle_pp_token_macro(Type, _) create_visitor(Type);
#include "require_pp/statement.h"
#include "require_pp/undefine.h"
    virtual ~StmtVisitor() = default;
  };
#undef create_visitor

  // Knowing what kind of expression a node represents is
  // highly useful for the analyzer, so we will implement one.
  struct ExprKindResolver: private ExprVisitor {
    // This class is only meant to provide `resolve`, since it
    // initializes `kind` in a well defined manner, this contraption is safe.
    ExprKindResolver(): kind{static_cast<kinds::Expression>(0)} {}
    // The visit overload sets the kind to member kind which
    // `resolve` will return assuming a successful setting.
    kinds::Expression resolve(Expr &expr);

  private:
    // Temporary holder of the expression kind.
    kinds::Expression kind;

#define rattle_undef_token_macro
#define rattle_pp_token_macro(Type, _)                                         \
  void visit(expr::Type &) { kind = kinds::Expression::Type; };
#include "require_pp/expression.h"
#include "require_pp/undefine.h"
  };
} // namespace rattle::tree::visitor

