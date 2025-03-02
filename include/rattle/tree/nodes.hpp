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

    struct UnaryExpr: Expr {
      UnaryExpr(token::Token op, utility::Scoped<Expr> operand)
        : op{op}, operand{std::move(operand)} {}
      token::Token op;
      utility::Scoped<Expr> operand;
      create_expr_visit;
    };

    // Quick way to check if a for loop is valid having an `in` expression
    struct In: BinaryExpr {
      // Cannot inherit constructor
      In(token::Token op, utility::Scoped<Expr> left,
        utility::Scoped<Expr> right)
        : BinaryExpr{op, std::move(left), std::move(right)} {}
    };

    // Good old if-else ternary operator: `ontrue if cond else onfalse`
    // It compares to c's conditional operator: `cond ? ontrue : onfalse`
    struct IfElse: Expr {
      IfElse(token::Token const &kw_if, token::Token const &kw_else,
        utility::Scoped<Expr> ontrue, utility::Scoped<Expr> cond,
        utility::Scoped<Expr> onfalse)
        : Expr{}, kw_if{kw_if}, kw_else{kw_else}, ontrue{std::move(ontrue)},
          cond{std::move(cond)}, onfalse{std::move(onfalse)} {}
      token::Token kw_if, kw_else;
      utility::Scoped<Expr> ontrue, cond, onfalse;
      create_expr_visit;
    };

    // Direct values; literals.
    struct Primary: Expr {
      Primary(token::Token const &value): Expr{}, value{value} {}
      token::Token value;
      create_expr_visit;
    };

    // Group expression also represents a tuple depending on held expr
    struct Group: Expr {
      Group(token::Token const &lparen, token::Token const &rparen,
        utility::Scoped<Expr> expr)
        : Expr{}, lparen{lparen}, rparen{rparen}, expr{std::move(expr)} {}
      token::Token lparen, rparen;
      utility::Scoped<Expr> expr;
      create_expr_visit;
    };
    // Anonymous function; syntax (|arg1, arg2| (3*arg1 + arg2) - 100)(50, 30) == 80
    struct Lambda: Expr {
      Lambda(token::Token const &lbar, token::Token const &rbar,
        utility::Scoped<Expr> params, utility::Scoped<Expr> body)
        : Expr{}, lbar{lbar}, rbar{rbar}, params{std::move(params)},
          body{std::move(body)} {}
      token::Token lbar, rbar;
      utility::Scoped<Expr> params, body;
    };
    // Call expression; also holds subscript.
    struct Call: Expr {
      Call(token::Token const &lparen, token::Token const &rparen,
        utility::Scoped<Expr> callable, utility::Scoped<Expr> params)
        : Expr{}, lparen{lparen}, rparen{rparen}, callable{std::move(callable)},
          params{std::move(params)} {}
      token::Token lparen, rparen;
      utility::Scoped<Expr> callable, params;
      create_expr_visit;
    };
    struct Dot: Expr {
      Dot(token::Token const &dot, utility::Scoped<Expr> expr,
        token::Token const &member)
        : dot(dot), member(member), expr(std::move(expr)) {}
      token::Token dot, member;
      utility::Scoped<Expr> expr;
      create_expr_visit;
    };
    struct IsNot: Expr {
      IsNot(token::Token const &is_, token::Token const &not_,
        utility::Scoped<Expr> left, utility::Scoped<Expr> right)
        : is_{is_}, not_{not_}, left{std::move(left)}, right{std::move(right)} {
      }
      token::Token is_, not_;
      utility::Scoped<Expr> left, right;
      create_expr_visit;
    };
    struct NotIn: Expr {
      NotIn(token::Token const &not_, token::Token const &in_,
        utility::Scoped<Expr> left, utility::Scoped<Expr> right)
        : not_{not_}, in_{in_}, left{std::move(left)}, right{std::move(right)} {
      }
      token::Token not_, in_;
      utility::Scoped<Expr> left, right;
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

  // Return a value from a function
  struct Return: Stmt {
    Return(token::Token const &tk, utility::Scoped<Expr> value)
      : Stmt{}, kw{tk}, value{std::move(value)} {}
    token::Token kw;
    utility::Scoped<Expr> value;
    create_stmt_visit;
  };

  // Loop control statements
  struct Continue: Stmt {
    Continue(token::Token const &tk): Stmt{}, kw{tk} {}
    token::Token kw;
    create_stmt_visit;
  };
  struct Break: Stmt {
    Break(token::Token const &tk): Stmt{}, kw{tk} {}
    token::Token kw;
    create_stmt_visit;
  };

  // A way to hold multiple nodes
  struct Block: Stmt {
    Block(token::Token const &lop, token::Token const &rop,
      std::vector<utility::Scoped<Stmt>> stmts)
      : Stmt{}, lop{lop}, rop{rop}, statements{std::move(stmts)} {}
    token::Token lop, rop;
    std::vector<utility::Scoped<Stmt>> statements;
    create_stmt_visit;
  };

  // Loop statements
  struct For: Stmt {
    For(token::Token const &tk, utility::Scoped<Expr> bind,
      utility::Scoped<Block> body)
      : Stmt{}, kw{tk}, binding{std::move(bind)}, body{std::move(body)} {}
    token::Token kw;
    utility::Scoped<Expr> binding;
    utility::Scoped<Block> body;
    create_stmt_visit;
  };
  struct While: Stmt {
    While(token::Token const &tk, utility::Scoped<Expr> cond,
      utility::Scoped<Block> body)
      : Stmt{}, kw{tk}, cond{std::move(cond)}, body{std::move(body)} {}
    token::Token kw;
    utility::Scoped<Expr> cond;
    utility::Scoped<Block> body;
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

  // Nodes that add names to scopes
  struct Def: Stmt {
    Def(token::Token const &tk, utility::Scoped<Expr> name_params,
      utility::Scoped<Block> body)
      : Stmt{}, kw{tk}, name_params{std::move(name_params)},
        body{std::move(body)} {}
    token::Token kw;
    utility::Scoped<Expr> name_params;
    utility::Scoped<Block> body;
    create_stmt_visit;
  }; // Declares a function

  struct Class: Stmt {
    Class(token::Token const &tk, utility::Scoped<Expr> name_bases,
      utility::Scoped<Block> body)
      : Stmt{}, kw{tk}, name_bases{std::move(name_bases)},
        body{std::move(body)} {}
    token::Token kw;
    utility::Scoped<Expr> name_bases;
    utility::Scoped<Block> body;
    create_stmt_visit;
  }; // Declares a class

  struct Global: Stmt {
    Global(token::Token const &tk, utility::Scoped<Expr> names)
      : Stmt{}, kw{tk}, names{std::move(names)} {}
    token::Token kw;
    utility::Scoped<Expr> names;
    create_stmt_visit;
  }; // Locks a local var with one in global scope of same name

  // Locks a local var with one in innermost wrapping,
  // not including global, scopes
  struct Nonlocal: Stmt {
    Nonlocal(token::Token const &tk, utility::Scoped<Expr> names)
      : Stmt{}, kw{tk}, names{std::move(names)} {}
    token::Token kw;
    utility::Scoped<Expr> names;
    create_stmt_visit;
  };
} // namespace rattle::tree

