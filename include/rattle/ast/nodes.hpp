#pragma once

#include "forward.h"
#include "node_kinds.hpp"
#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>
#include <string_view>
#include <utility>
#include <vector>

namespace rattle::ast {
  using utility::Scoped;

  struct Node {
    virtual ~Node() = default;
  };

  struct Stmt: Node {
    Stmt(Scoped<tree::Stmt> stmt): Node{}, raw{std::move(stmt)} {}
    Scoped<tree::Stmt> raw;
    virtual void visit(visitor::StmtVisitor &visitor) noexcept = 0;
  };

  struct Expr: Node {
    Expr(Scoped<tree::Expr> expr): Node{}, raw{std::move(expr)} {}
    Scoped<tree::Expr> raw;
    virtual void visit(visitor::ExprVisitor &visitor) noexcept = 0;
  };

#define create_visit(Visitor)                                                  \
  void visit(Visitor &v) noexcept override { v.visit(*this); }
#define create_expr_visit create_visit(visitor::ExprVisitor)
#define create_stmt_visit create_visit(visitor::StmtVisitor)

  namespace expr {
    struct Literal: Expr {
      Literal(kinds::Literal kind, std::string_view value,
        Scoped<tree::expr::Literal> raw)
        : Expr{std::move(raw)}, kind{kind}, value{value} {}
      kinds::Literal kind;
      std::string_view value;
      create_expr_visit;
    };
    struct BinaryExpr: Expr {
      BinaryExpr(kinds::BinaryExpr kind, Scoped<Expr> left, Scoped<Expr> right,
        Scoped<tree::expr::BinaryExpr> raw)
        : Expr{std::move(raw)}, kind{kind}, left{std::move(left)},
          right{std::move(right)} {}
      kinds::BinaryExpr kind;
      Scoped<Expr> left, right;
      create_expr_visit;
    };
    struct UnaryExpr: Expr {
      UnaryExpr(kinds::UnaryExpr kind, Scoped<Expr> operand,
        Scoped<tree::expr::BinaryExpr> raw)
        : Expr{std::move(raw)}, kind{kind}, operand{std::move(operand)} {}
      kinds::UnaryExpr kind;
      Scoped<Expr> operand;
      create_expr_visit;
    };
    struct TernaryExpr: Expr {
      TernaryExpr(kinds::TernaryExpr kind, Scoped<Expr> left,
        Scoped<Expr> middle, Scoped<Expr> right,
        Scoped<tree::expr::BinaryExpr> raw)
        : Expr{std::move(raw)}, kind{kind}, left{std::move(left)},
          middle{std::move(middle)}, right{std::move(right)} {}
      kinds::TernaryExpr kind;
      Scoped<Expr> left, middle, right;
      create_expr_visit;
    };
    struct Sequence: Expr {
      Sequence(
        kinds::Sequence kind, std::vector<Expr> values, Scoped<tree::Expr> raw)
        : Expr{std::move(raw)}, kind{kind} {}
      kinds::Sequence kind;
      std::vector<Expr> values;
      create_expr_visit;
    };
    struct Binding: Expr {
      struct Unit {
        Unit(kinds::Binding kind, std::string_view name, Scoped<tree::Expr> raw)
          : kind{kind}, name{name}, raw{std::move(raw)} {}
        kinds::Binding kind;
        std::string_view name;
        Scoped<tree::Expr> raw;
      };
      Binding(std::vector<Unit> bundle, Scoped<tree::expr::BinaryExpr> raw)
        : Expr{std::move(raw)}, bind{std::move(bundle)} {}
      std::vector<Unit> bind;
      create_expr_visit;
    };
    struct Lambda: Expr {
      Lambda(Scoped<Binding> params, Scoped<Expr> body,
        Scoped<tree::expr::BiExprBiTk> raw)
        : Expr{std::move(raw)}, parameters{std::move(params)},
          body{std::move(body)} {}
      Scoped<Binding> parameters;
      Scoped<Expr> body;
      create_expr_visit;
    };
  } // namespace expr

  namespace stmt {
    struct Block: Stmt {
      Block(std::vector<Scoped<Stmt>> stmts, Scoped<tree::stmt::Block> raw)
        : Stmt{std::move(raw)}, stmts{std::move(stmts)} {}
      std::vector<Scoped<Stmt>> stmts;
      create_stmt_visit;
    };

    struct ExprStmt: Stmt {
      ExprStmt(Scoped<Expr> expr, Scoped<tree::stmt::ExprStmt> raw)
        : Stmt{std::move(raw)}, expr{std::move(expr)} {}
      Scoped<Expr> expr;
      create_stmt_visit;
    };

    struct For: Stmt {
      For(Scoped<expr::Binding> binding, Scoped<Expr> iterable,
        Scoped<Stmt> body, Scoped<tree::stmt::TkExprBlock> raw)
        : Stmt{std::move(raw)}, binding{std::move(binding)},
          iterable{std::move(iterable)}, body{std::move(body)} {}
      Scoped<expr::Binding> binding;
      Scoped<Expr> iterable;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct While: Stmt {
      While(Scoped<Expr> condition, Scoped<Stmt> body,
        Scoped<tree::stmt::TkExprBlock> raw)
        : Stmt{std::move(raw)}, condition{std::move(condition)},
          body{std::move(body)} {}
      Scoped<Expr> condition;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct LoopControl: Stmt {
      LoopControl(kinds::LoopControl kind, Scoped<tree::stmt::TkExpr> raw)
        : Stmt{std::move(raw)}, kind{kind} {}
      kinds::LoopControl kind;
      create_stmt_visit;
    };

    struct Class: Stmt {
      Class(std::string_view name, Scoped<Stmt> body,
        Scoped<tree::stmt::TkExprBlock> raw)
        : Stmt{std::move(raw)}, name{name}, body{std::move(body)} {}
      std::string_view name;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct Def: Stmt {
      Def(kinds::Def kind, std::string_view name, Scoped<expr::Binding> params,
        Scoped<Stmt> body, Scoped<tree::stmt::TkExprBlock> raw)
        : Stmt{std::move(raw)}, kind{kind}, name{name},
          parameters{std::move(params)}, body{std::move(body)} {}
      kinds::Def kind;
      std::string_view name;
      Scoped<expr::Binding> parameters;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct Assignment: Stmt {
      Assignment(kinds::Assignment kind, Scoped<Expr> slot, Scoped<Expr> value,
        Scoped<tree::stmt::Assignment> raw)
        : Stmt{std::move(raw)}, kind{kind}, slot{std::move(slot)},
          value{std::move(value)} {}
      kinds::Assignment kind;
      Scoped<Expr> slot, value;
      create_stmt_visit;
    };

    struct Resolve: Stmt {
      Resolve(
        kinds::Resolve kind, Scoped<Expr> expr, Scoped<tree::stmt::TkExpr> raw)
        : Stmt{std::move(raw)}, kind{kind}, expr{std::move(expr)} {}
      kinds::Resolve kind;
      Scoped<Expr> expr;
      create_stmt_visit;
    };

    struct Return: Stmt {
      Return(Scoped<Expr> value, Scoped<tree::stmt::TkExpr> raw)
        : Stmt{std::move(raw)}, value{std::move(value)} {}
      Scoped<Expr> value;
      create_stmt_visit;
    };

    struct If: Stmt {
      If(Scoped<Expr> condition, Scoped<Stmt> ontrue, Scoped<Stmt> onfalse,
        Scoped<tree::stmt::If> raw)
        : Stmt{std::move(raw)}, condition{std::move(condition)},
          ontrue{std::move(ontrue)}, onfalse{std::move(onfalse)} {}
      Scoped<Expr> condition;
      Scoped<Stmt> ontrue, onfalse;
      create_stmt_visit;
    };
  } // namespace stmt
#undef create_visit
#undef create_stmt_visit
#undef create_visit
} // namespace rattle::ast

