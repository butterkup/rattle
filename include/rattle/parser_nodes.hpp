#pragma once

#include "category.hpp"
#include "lexer.hpp"
#include <memory>
#include <vector>

namespace rattle::parser::nodes {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  // Forward declarations of the nodes
#define TK_MACRO(Name, _) struct Name;
#define TK_INCLUDE                                                             \
  (TK_OPALL | TK_KEYUNARY | TK_KEYLITERAL | TK_KEYBINARY | TK_KEYOTHER |       \
   TK_ASSIGN | TK_PRINUMBER | TK_PRIMARY)
#include <rattle/token_macro.hpp>
  struct BinaryExpr;
  struct UnaryExpr;
  struct Statement;

  // Visitor ABC
#define CREATE_VISIT(Type) virtual void visit(Type &) = 0

  struct ExpressionVisitor {
#define TK_MACRO(Name, _) CREATE_VISIT(Name);
#define TK_INCLUDE                                                             \
  (TK_OPALL | TK_PRINUMBER | TK_PRIMARY | TK_KEYBINARY | TK_KEYUNARY |         \
   TK_KEYLITERAL)
#include "token_macro.hpp"

    // To communicate expression errors
    CREATE_VISIT(BinaryExpr);
    CREATE_VISIT(UnaryExpr);
  };

  struct StatementVisitor: ExpressionVisitor {
#define TK_MACRO(Name, _) CREATE_VISIT(Name);
#define TK_INCLUDE (TK_KEYOTHER | TK_ASSIGN)
#include "token_macro.hpp"

    // To communicate statement errors
    CREATE_VISIT(Statement);
  };

#undef CREATE_VISIT

  //
#define _node_visit(Visitor)                                                   \
  virtual void visit(Visitor &visitor) { visitor.visit(*this); }
#define _stmt _node_visit(StatementVisitor)
#define _expr _node_visit(ExpressionVisitor)

  struct Statement {
    lexer::Token token;
    Statement(lexer::Token const &token): token(token) {}
    virtual ~Statement() = default;
    _stmt;
  };

  struct Expression: Statement {
    Expression(lexer::Token const &op): Statement(op) {}
  };

  struct BinaryExpr: Expression {
    std::unique_ptr<Expression> left_operand, right_operand;
    BinaryExpr(lexer::Token const &operator_, std::unique_ptr<Expression> left,
               std::unique_ptr<Expression> right)
      : Expression(operator_), left_operand(std::move(left)),
        right_operand(std::move(right)) {}
    _expr;
  };

  struct UnaryExpr: Expression {
    std::unique_ptr<Expression> operand;
    UnaryExpr(lexer::Token const &operator_,
              std::unique_ptr<Expression> operand)
      : Expression(operator_), operand(std::move(operand)) {}
    _expr;
  };

  struct Block: Statement {
    std::vector<std::unique_ptr<Statement>> statements;
    Block(lexer::Token const &brace,
          std::vector<std::unique_ptr<Statement>> statements)
      : Statement(brace), statements(std::move(statements)) {}
    _stmt;
  };

  struct Break: Statement {
    Break(lexer::Token const &kwd): Statement(kwd) {}
    _stmt;
  };

  struct Continue: Statement {
    Continue(lexer::Token const &kwd): Statement(kwd) {}
    _stmt;
  };

  struct Else: Statement {
    std::unique_ptr<Block> body;
    Else(lexer::Token const &kwd, std::unique_ptr<Block> body)
      : Statement(kwd), body(std::move(body)) {}
    _stmt;
  };

  struct Lastly: Statement {
    std::unique_ptr<Block> body;
    Lastly(lexer::Token const &kwd, std::unique_ptr<Block> body)
      : Statement(kwd), body(std::move(body)) {}
    _stmt;
  };

  struct Except: Statement {
    std::unique_ptr<Expression> captured;
    std::unique_ptr<Block> body;
    Except(lexer::Token const &kwd, std::unique_ptr<Expression> captured,
           std::unique_ptr<Block> body)
      : Statement(kwd), captured(std::move(captured)), body(std::move(body)) {}
    _stmt;
  };

  struct Try: Statement {
    std::unique_ptr<Block> body_try;
    std::vector<std::unique_ptr<Except>> handlers;
    std::unique_ptr<Else> block_else;
    std::unique_ptr<Lastly> block_lastly;
    Try(lexer::Token const &kwd, std::vector<std::unique_ptr<Except>> handlers,
        std::unique_ptr<Else> block_else, std::unique_ptr<Lastly> block_lastly)
      : Statement(kwd), handlers(std::move(handlers)),
        block_else(std::move(block_else)),
        block_lastly(std::move(block_lastly)) {}
    _stmt;
  };

  struct Raise: Statement {
    std::unique_ptr<Expression> value;
    Raise(lexer::Token const &kwd, std::unique_ptr<Expression> value)
      : Statement(kwd), value(std::move(value)) {}
    _stmt;
  };

  struct Class: Statement {
    lexer::Token name;
    std::unique_ptr<Expression> bases;
    std::unique_ptr<Block> body;
    Class(lexer::Token const &kwd, lexer::Token const &name,
          std::unique_ptr<Expression> bases, std::unique_ptr<Block> body)
      : Statement(kwd), name(name), bases(std::move(bases)),
        body(std::move(body)) {}
    _stmt;
  };

  struct Fn: Statement {
    lexer::Token name;
    std::unique_ptr<Expression> params;
    std::unique_ptr<Block> body;
    Fn(lexer::Token const &kwd, lexer::Token const &name,
       std::unique_ptr<Expression> params, std::unique_ptr<Block> body)
      : Statement(kwd), name(name), params(std::move(params)),
        body(std::move(body)) {}
    _stmt;
  };

  struct Import: Statement {
    std::unique_ptr<Expression> module;
    Import(lexer::Token const &kwd, std::unique_ptr<Expression> module)
      : Statement(kwd), module(std::move(module)) {}
    _stmt;
  };

  struct From: Statement {
    std::unique_ptr<Expression> package;
    std::unique_ptr<Import> module;
    From(lexer::Token const &kwd, std::unique_ptr<Expression> package,
         std::unique_ptr<Import> module)
      : Statement(kwd), package(std::move(package)), module(std::move(module)) {
    }
    _stmt;
  };

  struct If: Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> block_if;
    std::unique_ptr<Else> block_else;
    If(lexer::Token const &kwd, std::unique_ptr<Expression> cond,
       std::unique_ptr<Block> block_if, std::unique_ptr<Else> block_else)
      : Statement(kwd), condition(std::move(cond)),
        block_if(std::move(block_if)), block_else(std::move(block_else)) {}
    _stmt;
  };

  struct While: Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
    While(lexer::Token const &kwd, std::unique_ptr<Expression> cond,
          std::unique_ptr<Block> body)
      : Statement(kwd), condition(std::move(cond)), body(std::move(body)) {}
    _stmt;
  };

  struct For: Statement {
    std::unique_ptr<Expression> bindings;
    std::unique_ptr<Block> body;
    For(lexer::Token const &kwd, std::unique_ptr<Expression> bindings,
        std::unique_ptr<Block> body)
      : Statement(kwd), bindings(std::move(bindings)), body(std::move(body)) {}
    _stmt;
  };

  struct Return: Statement {
    std::unique_ptr<Expression> value;
    Return(lexer::Token const &kwd, std::unique_ptr<Expression> value)
      : Statement(kwd), value(std::move(value)) {}
    _stmt;
  };

  struct With: Statement {
    std::unique_ptr<Expression> contexts;
    std::unique_ptr<Block> block;
    With(lexer::Token const &kwd, std::unique_ptr<Expression> ctx,
         std::unique_ptr<Block> block)
      : Statement(kwd), contexts(std::move(ctx)), block(std::move(block)) {}
    _stmt;
  };

  struct Assert: Statement {
    std::unique_ptr<Expression> condition;
    Assert(lexer::Token const &kwd, std::unique_ptr<Expression> cond)
      : Statement(kwd), condition(std::move(cond)) {}
    _stmt;
  };

  struct NonLocal: Statement {
    std::unique_ptr<Expression> identifiers;
    NonLocal(lexer::Token const &kwd, std::unique_ptr<Expression> ids)
      : Statement(kwd), identifiers(std::move(ids)) {}
    _stmt;
  };

  struct Global: Statement {
    std::unique_ptr<Expression> identifiers;
    Global(lexer::Token const &kwd, std::unique_ptr<Expression> ids)
      : Statement(kwd), identifiers(std::move(ids)) {}
    _stmt;
  };

  struct Del: Statement {
    std::unique_ptr<Expression> identifiers;
    Del(lexer::Token const &kwd, std::unique_ptr<Expression> ids)
      : Statement(kwd), identifiers(std::move(ids)) {}
    _stmt;
  };

  struct Assignment: Statement {
    std::unique_ptr<Expression> store, value;
    Assignment(lexer::Token const &op, std::unique_ptr<Expression> store,
               std::unique_ptr<Expression> value)
      : Statement(op), store(std::move(store)), value(std::move(value)) {}
  };

  struct Container: Expression {
    enum class Type { None, Error, Dict, List, Tuple, Group } type;
    std::unique_ptr<Expression> entries;
    Container(lexer::Token const &op, Type type,
              std::unique_ptr<Expression> entries)
      : Expression(op), type(type), entries(std::move(entries)) {}
    _stmt;
  };

#define INH_ASSIGN(_Name)                                                      \
  struct _Name: Assignment {                                                   \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> store,           \
          std::unique_ptr<Expression> value)                                   \
      : Assignment(op, std::move(store), std::move(value)) {}                  \
    _stmt;                                                                     \
  }

#define INH_BINARY_EXPR(_Name)                                                 \
  struct _Name: BinaryExpr {                                                   \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> left,            \
          std::unique_ptr<Expression> right)                                   \
      : BinaryExpr(op, std::move(left), std::move(right)) {}                   \
    _expr;                                                                     \
  }

#define INH_UNARY_EXPR(_Name)                                                  \
  struct _Name: UnaryExpr {                                                    \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> operand)         \
      : UnaryExpr(op, std::move(operand)) {}                                   \
    _expr;                                                                     \
  }

#define INH_LITERAL_EXPR(_Name)                                                \
  struct _Name: Expression {                                                   \
    _Name(lexer::Token const &op): Expression(op) {}                           \
    _expr;                                                                     \
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
#undef _node_visit
#undef _stmt
#undef _expr

#pragma GCC diagnostic pop
} // namespace rattle::parser::nodes

