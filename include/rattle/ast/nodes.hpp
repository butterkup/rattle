#pragma once

#include "node_kinds.hpp"
#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>
#include <string_view>
#include <utility>
#include <vector>

namespace rattle::ast {
  using utility::Scoped;

  struct Node {};
  struct Stmt: Node {};
  struct Expr: Node {};

  namespace expr {
    struct Literal: Expr {
      kinds::Literal kind;
      std::string_view value;
      Scoped<tree::expr::Literal> raw;
    };
    struct BinaryExpr: Expr {
      kinds::BinaryExpr kind;
      Scoped<Expr> left, right;
      Scoped<tree::Expr> raw;
    };
    struct UnaryExpr: Expr {
      kinds::UnaryExpr kind;
      Scoped<Expr> operand;
      Scoped<tree::expr::BinaryExpr> raw;
    };
    struct TernaryExpr: Expr {
      kinds::TernaryExpr kind;
      Scoped<tree::Expr> left, middle, right;
      Scoped<tree::expr::BinaryExpr> raw;
    };
    struct Sequence: Expr {
      kinds::Sequence kind;
      std::vector<Expr> values;
      Scoped<tree::Expr> raw;
    };
    struct Binding: Expr {
      struct Bundle {
        Bundle(
          kinds::Binding kind, std::string_view name, Scoped<tree::Expr> raw)
          : kind{kind}, name{name}, raw{std::move(raw)} {}
        kinds::Binding kind;
        std::string_view name;
        Scoped<tree::Expr> raw;
      };
      std::vector<Bundle> bind;
    };
    struct Lambda: Expr {
      Scoped<Binding> parameters;
      Scoped<Expr> body;
      Scoped<tree::expr::BiExprBiTk> raw;
    };
  } // namespace expr

  struct Block: Stmt {
    std::vector<Scoped<Stmt>> stmts;
    Scoped<tree::Block> raw;
  };

  struct ExprStmt: Stmt {
    Scoped<Expr> expr;
    Scoped<tree::ExprStmt> raw;
  };

  struct For: Stmt {
    Scoped<expr::Binding> binding;
    Scoped<Expr> iterable;
    Scoped<Stmt> body;
    Scoped<tree::TkExprBlock> raw;
  };

  struct While: Stmt {
    Scoped<Expr> condition;
    Scoped<Stmt> body;
    Scoped<tree::TkExprBlock> raw;
  };

  struct Class: Stmt {
    std::string_view name;
    Scoped<Stmt> body;
    Scoped<tree::TkExprBlock> raw;
  };

  struct Def: Stmt {
    std::string_view name;
    Scoped<expr::Binding> parameters;
    Scoped<Stmt> body;
    Scoped<tree::TkExprBlock> raw;
  };

  struct Assignment: Stmt {
    kinds::Assignment kind;
    Scoped<Expr> slot, value;
    Scoped<tree::Assignment> raw;
  };

  struct Resolve: Stmt {
    token::Token scope_kind;
    Scoped<expr::Binding> names;
    Scoped<tree::TkExpr> raw;
  };
} // namespace rattle::ast

