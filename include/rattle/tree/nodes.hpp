#pragma once

#include "api.hpp"
#include "nodes_kinds.h"
#include <cassert>
#include <optional>
#include <rattle/lexer.hpp>
#include <rattle/token/token.hpp>
#include <rattle/utility.hpp>
#include <utility>
#include <vector>

namespace rattle::tree {
  // An interface all nodes must present
  struct Node {
    virtual kinds::Node node_kind() const { return kinds::Node::None; }
    virtual ~Node() = default;
  };

  // Main types of nodes; they may also expose APIs
  // * Represent expression nodes
  struct Expr: Node {
    virtual void visit(visitor::ExprVisitor &) = 0;
    kinds::Node node_kind() const override { return kinds::Node::Expression; }
  };

  // * Represent statement nodes
  struct Stmt: Node {
    virtual void visit(visitor::StmtVisitor &) = 0;
    kinds::Node node_kind() const override { return kinds::Node::Statement; }
  };

#define create_visit(Visitor)                                                  \
  void visit(Visitor &visitor) override { visitor.visit(*this); }
#define create_stmt_visit create_visit(visitor::StmtVisitor)
#define create_expr_visit create_visit(visitor::ExprVisitor)

  namespace expr {
    struct BinaryExpr: Expr {
      BinaryExpr(token::Token op, utility::Scoped<Expr> left,
        utility::Scoped<Expr> right)
        : op{op}, left{std::move(left)}, right{std::move(right)} {}
      token::Token op;
      utility::Scoped<Expr> left, right;
      create_expr_visit;
    };

    // Quick way to check if a for loop is valid having an `in` expression
    struct In: BinaryExpr {
      // Cannot inherit constructor
      In(token::Token op, utility::Scoped<Expr> left,
        utility::Scoped<Expr> right)
        : BinaryExpr{op, std::move(left), std::move(right)} {}
    };

    struct Literal: Expr {
      Literal(token::Token const &value): Expr{}, value{value} {}
      token::Token value;
      create_expr_visit;
    };

    struct BiExprBiTk: Expr {
      BiExprBiTk(token::Token const &tk1, token::Token const &tk2,
        utility::Scoped<Expr> expr1, utility::Scoped<Expr> expr2)
        : Expr{}, tk1{tk1}, tk2{tk2}, expr1{std::move(expr1)},
          expr2{std::move(expr2)} {}
      token::Token tk1, tk2;
      utility::Scoped<Expr> expr1, expr2;
      create_expr_visit;
    };
  } // namespace expr

  // Bridge from Expr to Stmt
  struct ExprStmt: Stmt {
    ExprStmt(utility::Scoped<Expr> expr): Stmt{}, expr{std::move(expr)} {}
    utility::Scoped<Expr> expr;
    create_stmt_visit;
  };

  // Represents all kinds of assignment
  struct Assignment: Stmt {
    Assignment(token::Token const &tk, utility::Scoped<Expr> left,
      utility::Scoped<Expr> right)
      : Stmt{}, op{tk}, left{std::move(left)}, right{std::move(right)} {}
    token::Token op;
    utility::Scoped<Expr> left, right;
    create_stmt_visit;
  };

  struct TkExpr: Stmt {
    TkExpr(token::Token const &tk, utility::Scoped<Expr> expr)
      : tk{tk}, expr{std::move(expr)} {}
    token::Token tk;
    utility::Scoped<Expr> expr;
    create_stmt_visit;
  };

  struct Block: Stmt {
    Block(token::Token const &lop, token::Token const &rop,
      std::vector<utility::Scoped<Stmt>> stmts)
      : Stmt{}, lop{lop}, rop{rop}, statements{std::move(stmts)} {}
    token::Token lop, rop;
    std::vector<utility::Scoped<Stmt>> statements;
    create_stmt_visit;
  };

  struct TkExprBlock: Stmt {
    TkExprBlock(token::Token const &tk, utility::Scoped<Expr> expr,
      utility::Scoped<Block> block)
      : tk{tk}, expr{std::move(expr)}, block{std::move(block)} {}
    token::Token tk;
    utility::Scoped<Expr> expr;
    utility::Scoped<Block> block;
    create_stmt_visit;
  };

  namespace internal {
    struct Else {
      Else(token::Token const &tk, utility::Scoped<Block> body)
        : kw{tk}, body{std::move(body)} {}
      token::Token kw;
      utility::Scoped<Block> body;
    };
    struct If {
      If(token::Token const &tk, utility::Scoped<Expr> cond,
        utility::Scoped<Block> body)
        : kw{tk}, cond{std::move(cond)}, body{std::move(body)} {}
      token::Token kw;
      utility::Scoped<Expr> cond;
      utility::Scoped<Block> body;
    };
  } // namespace internal

  // Classic branch statement
  struct If: Stmt {
    If(internal::If if_, std::optional<internal::Else> else_)
      : Stmt{}, if_{std::move(if_)}, else_{std::move(else_)} {}
    internal::If if_;
    std::optional<internal::Else> else_;
    create_stmt_visit;
  };
} // namespace rattle::tree

