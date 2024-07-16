#pragma once

#include "base.hpp"
#include "../lexer/tokens.hpp"
#include <memory>
#include "expr.hpp"

namespace rat::parser::nodes {
#define ACCEPT                     \
  void accept(Visitor &visitor) {  \
    visitor.accept(*this);         \
  }

#define BinaryStmt(Klass)              \
  struct Klass: Expr {                 \
    lexer::Token op;                   \
    std::unique_ptr<Expr>left, right;  \
    Klass(std::unique_ptr<Expr> left,  \
      std::unique_ptr<Expr> right)     \
      : left(std::move(left)),         \
      right(std::move(right)) { }      \
    ACCEPT;                            \
  }

  BinaryStmt(GreaterEqual);
  BinaryStmt(SlashEqual);
  BinaryStmt(RshiftEqual);
  BinaryStmt(Equal);
  BinaryStmt(PlusEqual);
  BinaryStmt(LshiftEqual);
  BinaryStmt(MinusEqual);
  BinaryStmt(StarEqual);
  BinaryStmt(PercentEqual);
  BinaryStmt(LessEqual);
  BinaryStmt(InvertEqual);
  BinaryStmt(BitOrEqual);
  BinaryStmt(BitAndEqual);
  BinaryStmt(AtEqual);

  struct Return: Node {
    lexer::Token stmt;
    std::unique_ptr<Expr> value;
    Return(std::unique_ptr<Expr> value)
      : value(std::move(value)) { }
    ACCEPT;
  };

  struct Yield: Node {
    lexer::Token stmt;
    std::unique_ptr<Expr> value;
    Yield(std::unique_ptr<Expr> value)
      : value(std::move(value)) { }
    ACCEPT;
  };

#undef ACCEPT
#undef BinaryStmt
}
