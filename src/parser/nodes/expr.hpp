#pragma once

#include "base.hpp"
#include "../lexer/tokens.hpp"
#include <memory>

namespace rat::parser::nodes {
#define ACCEPT                     \
  void accept(ExprVisitor &visitor) {  \
    visitor.accept(*this);         \
  }

#define BinaryExpr(Klass)              \
  struct Klass: Expr {                 \
    lexer::Token op;                   \
    std::unique_ptr<Expr>left, right;  \
    Klass(lexer::Token op,             \
      std::unique_ptr<Expr> left,      \
      std::unique_ptr<Expr> right)     \
      : op(op), left(std::move(left)), \
      right(std::move(right)) { }      \
    ACCEPT;                            \
  }

#define UnaryExpr(Klass)                         \
  struct Klass: Expr {                           \
    lexer::Token op;                             \
    std::unique_ptr<Expr> argument;              \
    Klass(lexer::Token op,                       \
      std::unique_ptr<Expr> argument)            \
    : op(op), argument(std::move(argument)) { }  \
    ACCEPT;                                      \
  }

  BinaryExpr(Dot);
  BinaryExpr(Plus);
  BinaryExpr(Minus);
  BinaryExpr(Star);
  BinaryExpr(Percent);
  BinaryExpr(Slash);
  BinaryExpr(Less);
  BinaryExpr(Greater);
  BinaryExpr(Lshift);
  BinaryExpr(Rshift);
  BinaryExpr(BitOr);
  BinaryExpr(BitAnd);
  BinaryExpr(Invert);
  BinaryExpr(At);
  BinaryExpr(And);
  BinaryExpr(Or);
  BinaryExpr(In);
  BinaryExpr(NotIn);
  BinaryExpr(IsNot);
  BinaryExpr(NotEqual);
  BinaryExpr(EqualEqual);

  UnaryExpr(UnaryPlus);
  UnaryExpr(UnaryMinus);
  UnaryExpr(UnaryAt);
  UnaryExpr(Group);
  UnaryExpr(Not);

#undef ACCEPT
#undef BinaryExpr
#undef UnaryExpr
}
