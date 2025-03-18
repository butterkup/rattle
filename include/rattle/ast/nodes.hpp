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
    virtual void visit(visitor::StmtVisitor &visitor) noexcept = 0;
  };

  struct Expr: Node {
    virtual void visit(visitor::ExprVisitor &visitor) noexcept = 0;
  };

#define create_visit(Visitor)                                                  \
  void visit(Visitor &v) noexcept override { v.visit(*this); }
#define create_expr_visit create_visit(visitor::ExprVisitor)
#define create_stmt_visit create_visit(visitor::StmtVisitor)

  namespace expr {
    struct Literal: Expr {
      Literal(kinds::Literal kind, std::string_view value,
        tree::expr::Literal *raw = nullptr)
        : Expr{}, raw{raw}, kind{kind}, value{value} {}
      tree::expr::Literal *raw;
      kinds::Literal kind;
      std::string_view value;
      create_expr_visit;
    };
    struct BinaryExpr: Expr {
      BinaryExpr(kinds::BinaryExpr kind, Scoped<Expr> left, Scoped<Expr> right,
        tree::Expr *raw = nullptr)
        : Expr{}, raw{raw}, kind{kind}, left{std::move(left)},
          right{std::move(right)} {}
      tree::Expr *raw;
      kinds::BinaryExpr kind;
      Scoped<Expr> left, right;
      create_expr_visit;
    };
    struct Binding: Expr {
      Binding(
        kinds::Binding kind, std::string_view name, tree::Expr *raw = nullptr)
        : raw{raw}, kind{kind}, name{name} {}
      tree::Expr *raw;
      kinds::Binding kind;
      std::string_view name;
      create_expr_visit;
    };
    struct UnaryExpr: Expr {
      UnaryExpr(
        kinds::UnaryExpr kind, Scoped<Expr> operand, tree::Expr *raw = nullptr)
        : Expr{}, raw{raw}, kind{kind}, operand{std::move(operand)} {}
      tree::Expr *raw;
      kinds::UnaryExpr kind;
      Scoped<Expr> operand;
      create_expr_visit;
    };
    struct TernaryExpr: Expr {
      TernaryExpr(kinds::TernaryExpr kind, Scoped<Expr> left,
        Scoped<Expr> middle, Scoped<Expr> right,
        tree::expr::BiExprBiTk *raw = nullptr)
        : Expr{}, raw{raw}, kind{kind}, left{std::move(left)},
          middle{std::move(middle)}, right{std::move(right)} {}
      tree::expr::BiExprBiTk *raw;
      kinds::TernaryExpr kind;
      Scoped<Expr> left, middle, right;
      create_expr_visit;
    };
    struct Lambda: Expr {
      Lambda(std::vector<Scoped<Binding>> params, Scoped<Expr> body,
        tree::expr::BiExprBiTk *raw = nullptr)
        : Expr{}, raw{raw}, parameters{std::move(params)},
          body{std::move(body)} {}
      tree::expr::BiExprBiTk *raw;
      std::vector<Scoped<Binding>> parameters;
      Scoped<Expr> body;
      create_expr_visit;
    };
  } // namespace expr

  namespace stmt {
    struct Event: Stmt {
      Event(kinds::Event kind, tree::Stmt *raw = nullptr): Stmt{}, raw{raw} {}
      tree::Stmt *raw;
      kinds::Event kind;
      create_stmt_visit;
    };

    struct ExprStmt: Stmt {
      ExprStmt(Scoped<Expr> expr, tree::stmt::ExprStmt *raw = nullptr)
        : Stmt{}, raw{raw}, expr{std::move(expr)} {}
      tree::stmt::ExprStmt *raw;
      Scoped<Expr> expr;
      create_stmt_visit;
    };

    struct For: Stmt {
      For(Scoped<Expr> binding, Scoped<Expr> iterable, Scoped<Stmt> body,
        tree::stmt::TkExprStmt *raw = nullptr)
        : Stmt{}, raw{raw}, binding{std::move(binding)},
          iterable{std::move(iterable)}, body{std::move(body)} {}
      tree::stmt::TkExprStmt *raw;
      Scoped<Expr> binding, iterable;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct While: Stmt {
      While(Scoped<Expr> condition, Scoped<Stmt> body,
        tree::stmt::TkExprStmt *raw = nullptr)
        : Stmt{}, raw{raw}, condition{std::move(condition)},
          body{std::move(body)} {}
      tree::stmt::TkExprStmt *raw;
      Scoped<Expr> condition;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct Class: Stmt {
      Class(std::string_view name, Scoped<Stmt> body,
        tree::stmt::TkExprStmt *raw = nullptr)
        : Stmt{}, raw{raw}, name{name}, body{std::move(body)} {}
      tree::stmt::TkExprStmt *raw;
      std::string_view name;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct Def: Stmt {
      Def(kinds::Def kind, std::string_view name, Scoped<Expr> params,
        Scoped<Stmt> body, tree::stmt::TkExprStmt *raw = nullptr)
        : Stmt{}, raw{raw}, kind{kind}, name{name},
          parameters{std::move(params)}, body{std::move(body)} {}
      tree::stmt::TkExprStmt *raw;
      kinds::Def kind;
      std::string_view name;
      Scoped<Expr> parameters;
      Scoped<Stmt> body;
      create_stmt_visit;
    };

    struct Assignment: Stmt {
      Assignment(kinds::Assignment kind, Scoped<Expr> slot, Scoped<Expr> value,
        tree::stmt::Assignment *raw = nullptr)
        : Stmt{}, raw{raw}, kind{kind}, slot{std::move(slot)},
          value{std::move(value)} {}
      tree::stmt::Assignment *raw;
      kinds::Assignment kind;
      Scoped<Expr> slot, value;
      create_stmt_visit;
    };

    struct Command: Stmt {
      Command(kinds::Command kind, Scoped<Expr> expr,
        tree::stmt::TkExpr *raw = nullptr)
        : Stmt{}, raw{raw}, kind{kind}, expr{std::move(expr)} {}
      tree::stmt::TkExpr *raw;
      kinds::Command kind;
      Scoped<Expr> expr;
      create_stmt_visit;
    };

    struct If: Stmt {
      If(Scoped<Expr> condition, Scoped<Stmt> ontrue,
        tree::stmt::TkExprStmt *raw = nullptr)
        : Stmt{}, raw{raw}, condition{std::move(condition)},
          ontrue{std::move(ontrue)} {}
      tree::stmt::TkExprStmt *raw;
      Scoped<Expr> condition;
      Scoped<Stmt> ontrue;
      create_stmt_visit;
    };

    struct Else: Stmt {
      Else(Scoped<Stmt> onfalse, tree::stmt::TkExprStmt *raw = nullptr)
        : raw{raw}, onfalse{std::move(onfalse)} {}
      tree::stmt::TkExprStmt *raw;
      Scoped<Stmt> onfalse;
      create_stmt_visit;
    };
  } // namespace stmt
#undef create_visit
#undef create_stmt_visit
#undef create_visit
} // namespace rattle::ast

