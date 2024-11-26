#pragma once

#include "category.hpp"
#include "lexer.hpp"
#include <memory>
#include <vector>

namespace rattle::parser::nodes {
  struct Statement {};

  struct Error: Statement {};

  struct Expression: Statement {
    lexer::Token operator_;
  };

  struct BinaryExpr: Expression {
    Expression left_operand, right_operand;
  };

  struct UnaryExpr: Expression {
    Expression operand;
  };

  struct As: Statement {
    std::unique_ptr<Expression> expr;
    lexer::Token alias;
  };

  struct Block: Statement {
    std::vector<std::unique_ptr<Statement>> statements;
  };

  struct Else: Block {};
  struct Lastly: Block {};

  struct Except: Statement {
    std::unique_ptr<As> captured;
    Block body;
  };

  struct Try: Statement {
    Block body_try;
    std::vector<Except> handlers;
    Else block_else;
    Lastly block_lastly;
  };

  struct Raise: Statement {
    std::unique_ptr<Expression> target;
  };

  struct Class: Statement {
    std::unique_ptr<Expression> base;
    lexer::Token name;
    Block body;
  };

  struct Fn: Statement {
    lexer::Token name;
    Block body;
  };

  struct Import: Statement {
    std::vector<lexer::Token> path_module;
    lexer::Token alias;
  };

  struct From: Import {
    std::vector<lexer::Token> path_package;
  };

  struct If: Statement {
    std::unique_ptr<Expression> condition;
    Block block_if;
    Else block_else;
  };

  struct While: Statement {
    std::unique_ptr<Expression> condition;
    Block block;
  };

  struct For: Statement {
    lexer::Token alias;
    std::unique_ptr<Expression> iterable;
  };

  struct Return: Statement {
    std::unique_ptr<Expression> value;
  };

  struct Yield: Statement {
    std::unique_ptr<Expression> value;
  };

  struct With: Statement {
    std::vector<As> contexts;
    Block block;
  };

  struct Assert: Statement {
    std::unique_ptr<Expression> condition, message;
  };

  struct NonLocal: Statement {
    std::vector<lexer::Token> identifiers;
  };

  struct Global: Statement {
    std::vector<lexer::Token> identifiers;
  };

  struct Del: Statement {
    std::vector<lexer::Token> identifiers;
  };

  struct Assignment: Statement {
    lexer::Token operator_;
    std::unique_ptr<Expression> assigned, assignee;
  };

  struct LiteralExpr: Expression {};

#define CREATE_NODE(Name, Base)                                                \
  struct Name: Base {};

#define TK_MACRO(Name, _) CREATE_NODE(Name, Assignment)
#define TK_INCLUDE TK_ASSIGN
#include "token_macro.hpp"

#define TK_MACRO(Name, _) CREATE_NODE(Name, LiteralExpr)
#define TK_INCLUDE (TK_PRI_NUMBER | TK_PRIMARY | TK_KEYLITERAL)
#include "token_macro.hpp"

#define TK_MACRO(Name, _) CREATE_NODE(Name, BinaryExpr)
#define TK_INCLUDE (TK_KEYBINARY | TK_OPALL)
#include "token_macro.hpp"

#define TK_MACRO(Name, _) CREATE_NODE(Name, UnaryExpr)
#define TK_INCLUDE TK_KEYUNARY
#include "token_macro.hpp"

  struct UPlus: UnaryExpr {};
  struct UMinus: UnaryExpr {};
  struct Group: UnaryExpr {};
  struct Separator: BinaryExpr {};

#undef CREATE_NODE

  struct AnonFn: Expression {
    std::unique_ptr<Expression> parameters, body;
  };

  // Visitor ABC

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#define CREATE_VISIT(Type) virtual void visit(Type &) = 0

  struct ExpressionVisitor {
#define TK_MACRO(Name, _) CREATE_VISIT(Name);
#define TK_INCLUDE (TK_OPALL | TK_KEYBINARY | TK_KEYUNARY | TK_KEYLITERAL)
#include "token_macro.hpp"

    CREATE_VISIT(UPlus);
    CREATE_VISIT(UMinus);
    CREATE_VISIT(Group);
    CREATE_VISIT(Separator);
    CREATE_VISIT(AnonFn);
  };

  struct StatementVisitor: ExpressionVisitor {
#define TK_MACRO(Name, _) CREATE_VISIT(Name);
#define TK_INCLUDE (TK_KEYOTHER | TK_ASSIGN)
#include "token_macro.hpp"
  };

#undef CREATE_VISIT

#pragma GCC diagnostic pop
} // namespace rattle::parser::nodes

