#pragma once

#include "api.hpp"
#include <cassert>
#include <optional>
#include <rattle/lexer.hpp>
#include <rattle/token/token.hpp>
#include <rattle/utility.hpp>
#include <utility>
#include <vector>

namespace rattle::tree {
  using utility::Scoped;
  // An interface all nodes must present
  struct Node {
    virtual ~Node() = default;
  };

  // Main types of nodes; they may also expose APIs
  // * Represent expression nodes
  struct Expr: Node {
    virtual void visit(visitor::Expression &) noexcept = 0;
  };

  // * Represent statement nodes
  struct Stmt: Node {
    virtual void visit(visitor::Statement &) noexcept = 0;
  };

#define create_visit(Visitor)                                                  \
  void visit(Visitor &visitor) noexcept override { visitor.visit(*this); }
#define create_stmt_visit create_visit(visitor::Statement)
#define create_expr_visit create_visit(visitor::Expression)

  namespace expr {
    struct UnaryExpr: Expr {
      UnaryExpr(token::Token op, Scoped<Expr> operand)
        : op{op}, operand{std::move(operand)} {}
      token::Token op;
      Scoped<Expr> operand;
      create_expr_visit;
    };

    struct BinaryExpr: Expr {
      BinaryExpr(token::Token op, Scoped<Expr> left, Scoped<Expr> right)
        : op{op}, left{std::move(left)}, right{std::move(right)} {}
      token::Token op;
      Scoped<Expr> left, right;
      create_expr_visit;
    };

    struct Literal: Expr {
      Literal(token::Token const &value): Expr{}, value{value} {}
      token::Token value;
      create_expr_visit;
    };

    struct BiExprBiTk: Expr {
      BiExprBiTk(token::Token const &tk1, token::Token const &tk2,
        Scoped<Expr> expr1, Scoped<Expr> expr2)
        : Expr{}, tk1{tk1}, tk2{tk2}, expr1{std::move(expr1)},
          expr2{std::move(expr2)} {}
      token::Token tk1, tk2;
      Scoped<Expr> expr1, expr2;
      create_expr_visit;
    };
  } // namespace expr

  namespace stmt {
    // Bridge from Expr to Stmt
    struct ExprStmt: Stmt {
      ExprStmt(Scoped<Expr> expr): Stmt{}, expr{std::move(expr)} {}
      Scoped<Expr> expr;
      create_stmt_visit;
    };

    // Represents all kinds of assignment
    struct Assignment: Stmt {
      Assignment(token::Token const &tk, Scoped<Expr> left, Scoped<Expr> right)
        : Stmt{}, op{tk}, left{std::move(left)}, right{std::move(right)} {}
      token::Token op;
      Scoped<Expr> left, right;
      create_stmt_visit;
    };

    struct TkExpr: Stmt {
      TkExpr(token::Token const &tk, Scoped<Expr> expr)
        : tk{tk}, expr{std::move(expr)} {}
      token::Token tk;
      Scoped<Expr> expr;
      create_stmt_visit;
    };

    struct Block: Stmt {
      Block(token::Token const &lop, token::Token const &rop,
        std::vector<Scoped<Stmt>> stmts)
        : Stmt{}, lop{lop}, rop{rop}, statements{std::move(stmts)} {}
      token::Token lop, rop;
      std::vector<Scoped<Stmt>> statements;
      create_stmt_visit;
    };

    struct TkExprBlock: Stmt {
      TkExprBlock(
        token::Token const &tk, Scoped<Expr> expr, Scoped<Block> block)
        : tk{tk}, expr{std::move(expr)}, block{std::move(block)} {}
      token::Token tk;
      Scoped<Expr> expr;
      Scoped<Block> block;
      create_stmt_visit;
    };

    namespace internal {
      struct Else {
        Else(token::Token const &tk, Scoped<Stmt> body)
          : kw{tk}, body{std::move(body)} {}
        token::Token kw;
        Scoped<Stmt> body;
      };
      struct If {
        If(token::Token const &tk, Scoped<Expr> cond, Scoped<Block> body)
          : kw{tk}, cond{std::move(cond)}, body{std::move(body)} {}
        token::Token kw;
        Scoped<Expr> cond;
        Scoped<Block> body;
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
  } // namespace stmt
#undef create_visit
#undef create_stmt_visit
#undef create_expr_visit
} // namespace rattle::tree

