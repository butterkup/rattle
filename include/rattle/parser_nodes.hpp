#pragma once

#include "category.hpp"
#include "lexer.hpp"
#include <memory>
#include <vector>

namespace rattle::parser::nodes {
  struct Statement {};

  struct Expression: Statement {
    lexer::Token operator_;
    Expression(lexer::Token const &op): Statement(), operator_(op) {}
  };

  struct BinaryExpr: Expression {
    std::unique_ptr<Expression> left_operand, right_operand;
    BinaryExpr(lexer::Token const &operator_, std::unique_ptr<Expression> left,
               std::unique_ptr<Expression> right)
      : Expression(operator_), left_operand(std::move(left)),
        right_operand(std::move(right)) {}
  };

  struct UnaryExpr: Expression {
    std::unique_ptr<Expression> operand;

    UnaryExpr(lexer::Token const &operator_,
              std::unique_ptr<Expression> operand)
      : Expression(operator_), operand(std::move(operand)) {}
  };

  struct As: Statement {
    std::unique_ptr<Expression> expr;
    lexer::Token alias;
  };

  struct Block: Statement {
    std::vector<std::unique_ptr<Statement>> statements;
    Block(std::vector<std::unique_ptr<Statement>> statements)
      : Statement(), statements(std::move(statements)) {}
  };

  struct Break: Statement {
    lexer::Token identifier;
  };

  struct Continue: Statement {
    lexer::Token identifier;
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
    std::unique_ptr<Expression> value;
    Raise(std::unique_ptr<Expression> value)
      : Statement(), value(std::move(value)) {}
  };

  struct Class: Statement {
    lexer::Token name;
    std::unique_ptr<Expression> bases;
    std::unique_ptr<Block> body;
    Class(lexer::Token const &name, std::unique_ptr<Expression> bases,
          std::unique_ptr<Block> body)
      : Statement(), name(name), bases(std::move(bases)),
        body(std::move(body)) {}
  };

  struct Fn: Statement {
    lexer::Token name;
    std::unique_ptr<Expression> params;
    std::unique_ptr<Block> body;
    Fn(lexer::Token const &name, std::unique_ptr<Expression> params,
       std::unique_ptr<Block> body)
      : Statement(), name(name), params(std::move(params)),
        body(std::move(body)) {}
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
    While(std::unique_ptr<Expression> cond, Block body)
      : Statement(), condition(std::move(cond)), block(std::move(body)) {}
  };

  struct For: Statement {
    lexer::Token alias;
    std::unique_ptr<Expression> iterable;
    For(lexer::Token const &alias, std::unique_ptr<Expression> iterable)
      : Statement(), alias(alias), iterable(std::move(iterable)) {}
  };

  struct Return: Statement {
    std::unique_ptr<Expression> value;
    Return(std::unique_ptr<Expression> value)
      : Statement(), value(std::move(value)) {}
  };

  struct With: Statement {
    std::vector<std::unique_ptr<As>> contexts;
    Block block;
    With(std::vector<std::unique_ptr<As>> ctx, Block body)
      : Statement(), contexts(std::move(ctx)), block(std::move(body)) {}
  };

  struct Assert: Statement {
    std::unique_ptr<Expression> condition, message;
    Assert(std::unique_ptr<Expression> cond, std::unique_ptr<Expression> msg)
      : Statement(), condition(std::move(cond)), message(std::move(msg)) {}
  };

  struct NonLocal: Statement {
    std::vector<lexer::Token> identifiers;
    NonLocal(std::vector<lexer::Token> ids)
      : Statement(), identifiers(std::move(ids)) {}
  };

  struct Global: Statement {
    std::vector<lexer::Token> identifiers;
    Global(std::vector<lexer::Token> ids)
      : Statement(), identifiers(std::move(ids)) {}
  };

  struct Del: Statement {
    std::vector<lexer::Token> identifiers;
    Del(std::vector<lexer::Token> ids)
      : Statement(), identifiers(std::move(ids)) {}
  };

  struct Assignment: Statement {
    lexer::Token operator_;
    std::unique_ptr<Expression> assignable, assignee;
  };

  struct LiteralExpr: Expression {};

  struct Container: LiteralExpr {
    enum class Type { None, Error, Dict, List, Tuple, Group } type;
    std::unique_ptr<Expression> entries;
    Container(lexer::Token const &op, Type type,
              std::unique_ptr<Expression> entries)
      : LiteralExpr({op}), type(type), entries(std::move(entries)) {}
  };

#define INH_ASSIGN(_Name)                                                      \
  struct _Name: Assignment {                                                   \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> assignable,      \
          std::unique_ptr<Expression> assignee)                                \
      : Assignment({}, op, std::move(assignable), std::move(assignee)) {}      \
  }

#define INH_BINARY_EXPR(_Name)                                                 \
  struct _Name: BinaryExpr {                                                   \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> left,            \
          std::unique_ptr<Expression> right)                                   \
      : BinaryExpr(op, std::move(left), std::move(right)) {}                   \
  }

#define INH_UNARY_EXPR(_Name)                                                  \
  struct _Name: UnaryExpr {                                                    \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> operand)         \
      : UnaryExpr(op, std::move(operand)) {}                                   \
  }

#define INH_LITERAL_EXPR(_Name)                                                \
  struct _Name: LiteralExpr {                                                  \
    _Name(lexer::Token const &op): LiteralExpr({op}) {}                        \
  }

#define TK_MACRO(Name, _) INH_ASSIGN(Name);
#define TK_INCLUDE TK_ASSIGN
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_LITERAL_EXPR(Name);
#define TK_INCLUDE (TK_PRINUMBER | TK_PRIMARY | TK_KEYLITERAL)
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_BINARY_EXPR(Name);
#define TK_INCLUDE TK_PRIBINARY
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_BINARY_EXPR(Name);
#define TK_INCLUDE (TK_KEYBINARY | TK_OPALL)
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_UNARY_EXPR(Name);
#define TK_INCLUDE TK_KEYUNARY
#include "token_macro.hpp"

#undef INH_LITERAL_EXPR
#undef INH_ASSIGN
#undef INH_UNARY_EXPR
#undef INH_BINARY_EXPR

  // Visitor ABC

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#define CREATE_VISIT(Type) virtual void visit(Type &) = 0

  struct ExpressionVisitor {
#define TK_MACRO(Name, _) CREATE_VISIT(Name);
#define TK_INCLUDE (TK_OPALL | TK_KEYBINARY | TK_KEYUNARY | TK_KEYLITERAL)
#include "token_macro.hpp"

    // To communicate expression errors
    CREATE_VISIT(BinaryExpr);
    CREATE_VISIT(UnaryExpr);
  };

  struct StatementVisitor: ExpressionVisitor {
#define TK_MACRO(Name, _) CREATE_VISIT(Name);
#define TK_INCLUDE (TK_KEYOTHER | TK_ASSIGN)
#include "token_macro.hpp"
  };

#undef CREATE_VISIT

#pragma GCC diagnostic pop
} // namespace rattle::parser::nodes

