#pragma once

#include "api.hpp"
#include "nodes_kinds.h"
#include <cassert>
#include <memory>
#include <rattle/lexer.hpp>
#include <rattle/token/token.hpp>
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
      token::Token op;
      std::unique_ptr<Expr> left, right;
      BinaryExpr(token::Token op, std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right)
        : op{op}, left{std::move(left)}, right{std::move(right)} {}
    };

    struct UnaryExpr: Expr {
      token::Token op;
      std::unique_ptr<Expr> operand;
      UnaryExpr(token::Token op, std::unique_ptr<Expr> operand)
        : op{op}, operand{std::move(operand)} {}
    };

    // Auto generate expression nodes:
    // * binary operators
#define rattle_undef_token_macro
#define rattle_pp_token_macro(kind, _)                                         \
  struct kind: BinaryExpr {                                                    \
    kind(token::Token op, std::unique_ptr<Expr> left,                          \
      std::unique_ptr<Expr> right)                                             \
      : BinaryExpr{op, std::move(left), std::move(right)} {}                   \
    create_expr_visit                                                          \
  };
#include "require_pp/binary.h"
#include "require_pp/undefine.h"

    // * unary operators
#define rattle_undef_token_macro
#define rattle_pp_token_macro(kind, _)                                         \
  struct kind: UnaryExpr {                                                     \
    kind(token::Token op, std::unique_ptr<Expr> operand)                       \
      : UnaryExpr{op, std::move(operand)} {}                                   \
    create_expr_visit                                                          \
  };
#include "require_pp/unary.h"
#include "require_pp/undefine.h"

    // Good old if-else ternary operator: `ontrue if cond else onfalse`
    // It compares to c's conditional operator: `cond ? ontrue : onfalse`
    struct IfElse: Expr {
      token::Token kw_if, kw_else;
      std::unique_ptr<Expr> ontrue, cond, onfalse;
      create_expr_visit;
    };

    // Direct values; literals.
    struct Primary: Expr {
      token::Token value;
      create_expr_visit;
    };

    // Group expression; sole purpose is to
    // raise the precedence of held expression.
    struct Group: Expr {
      token::Token lparen, rparen;
      std::unique_ptr<Expr> expr;
      create_expr_visit;
    };
  } // namespace expr

  // Bridge from Expr to Stmt
  struct ExprStmt: Stmt {
    std::unique_ptr<Expr> expr;
    create_stmt_visit;
  };

  // Represents all kinds of assignment
  struct Assignment: Stmt {
    kinds::Assignment kind;
    token::Token op;
    std::unique_ptr<Expr> left, right;
    create_stmt_visit;
  };

  // Return a value from a function
  struct Return: Stmt {
    token::Token kw;
    std::unique_ptr<Expr> value;
    create_stmt_visit;
  };

  // Loop control statements
  struct Continue: Stmt {
    token::Token kw;
    create_stmt_visit;
  };
  struct Break: Stmt {
    token::Token kw;
    create_stmt_visit;
  };

  // A way to hold multiple nodes
  struct Block: Stmt {
    token::Token lop, rop;
    std::vector<std::unique_ptr<Stmt>> statements;
    create_stmt_visit;
  };

  // Loop statements
  struct For: Stmt {
    token::Token kw;
    std::unique_ptr<Expr> binding;
    std::unique_ptr<Block> body;
    create_stmt_visit;
  };
  struct While: Stmt {
    token::Token kw;
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Block> body;
    create_stmt_visit;
  };

  // Lonely `else`
  struct Else: Stmt {
    token::Token kw;
    std::unique_ptr<Block> body;
    create_stmt_visit;
  };

  // Classic branch statement
  struct If: Stmt {
    token::Token kw;
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Block> body;
    create_stmt_visit;
  };

  struct ElseIf: Stmt {
    token::Token kw_else;
    std::unique_ptr<If> if_;
    create_stmt_visit;
  };

  // IfElse
  struct IfElse: Stmt {
    std::unique_ptr<If> if_;
    std::vector<std::unique_ptr<ElseIf>> else_if_;
    std::unique_ptr<Else> else_;
    create_stmt_visit;
  };

  // Nodes that add names to scopes
  struct Def: Stmt {
    token::Token kw, name;
    std::unique_ptr<Expr> params;
    std::unique_ptr<Block> body;
    create_stmt_visit;
  }; // Declares a function

  struct Class: Stmt {
    token::Token kw, name;
    std::unique_ptr<Block> body;
    create_stmt_visit;
  }; // Declares a class

  struct Global: Stmt {
    token::Token kw;
    std::unique_ptr<Expr> names;
    create_stmt_visit;
  }; // Locks a local var with one in global scope of same name

  // Locks a local var with one in innermost wrapping,
  // not including global, scopes
  struct Nonlocal: Stmt {
    token::Token kw;
    std::unique_ptr<Expr> names;
    create_stmt_visit;
  };
} // namespace rattle::tree

