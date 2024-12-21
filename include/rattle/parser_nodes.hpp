#pragma once

#ifndef RATTLE_SOURCE_ONLY
#include "category.hpp"
#include "lexer.hpp"
#include <memory>
#include <vector>
#endif

namespace rattle::parser::nodes {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  // Forward declarations of the nodes
#define TK_MACRO(Name, _) struct Name;
#define TK_INCLUDE TK_ALL_NODES
#include <rattle/token_macro.hpp>

  // Visitor ABC
  struct Visitor {
#define TK_MACRO(Name, _) virtual void visit(Name &) = 0;
#define TK_INCLUDE TK_ALL_NODES
#include "token_macro.hpp"
  };

  // Virtual node visitor methods
#define _node_visit(qualifier)                                                 \
  virtual void visit(Visitor &visitor) qualifier { visitor.visit(*this); }
#define _visit _node_visit(override)

  // Node implementations
  struct Statement {
    lexer::Token token;
    Statement(lexer::Token const &token): token(token) {}
    virtual ~Statement() = default;
    _node_visit();
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
    _visit;
  };

  struct UnaryExpr: Expression {
    std::unique_ptr<Expression> operand;
    UnaryExpr(lexer::Token const &operator_,
              std::unique_ptr<Expression> operand)
      : Expression(operator_), operand(std::move(operand)) {}
    _visit;
  };

  struct IfElse: BinaryExpr {
    std::unique_ptr<Expression> condition;
    IfElse(lexer::Token const &tk, std::unique_ptr<Expression> cond,
           std::unique_ptr<Expression> if_, std::unique_ptr<Expression> else_)
      : BinaryExpr(tk, std::move(if_), std::move(else_)),
        condition(std::move(cond)) {}
    _visit;
  };

  struct ExprStatement: Statement {
    std::unique_ptr<Expression> expr;
    ExprStatement(lexer::Token const &eos, std::unique_ptr<Expression> expr)
      : Statement(eos), expr(std::move(expr)) {}
    _visit;
  };

  struct Block: Statement {
    std::vector<std::unique_ptr<Statement>> statements;
    Block(lexer::Token const &brace,
          std::vector<std::unique_ptr<Statement>> statements)
      : Statement(brace), statements(std::move(statements)) {}
    _visit;
  };

  struct Break: Statement {
    Break(lexer::Token const &kwd): Statement(kwd) {}
    _visit;
  };

  struct Continue: Statement {
    Continue(lexer::Token const &kwd): Statement(kwd) {}
    _visit;
  };

  struct Else: Statement {
    std::unique_ptr<Statement> body;
    Else(lexer::Token const &kwd, std::unique_ptr<Statement> body)
      : Statement(kwd), body(std::move(body)) {}
    _visit;
  };

  struct Lastly: Statement {
    std::unique_ptr<Statement> body;
    Lastly(lexer::Token const &kwd, std::unique_ptr<Statement> body)
      : Statement(kwd), body(std::move(body)) {}
    _visit;
  };

  struct Except: Statement {
    std::unique_ptr<Expression> captured;
    std::unique_ptr<Statement> body;
    Except(lexer::Token const &kwd, std::unique_ptr<Expression> captured,
           std::unique_ptr<Statement> body)
      : Statement(kwd), captured(std::move(captured)), body(std::move(body)) {}
    _visit;
  };

  struct Try: Statement {
    std::unique_ptr<Statement> body_try;
    std::vector<std::unique_ptr<Except>> handlers;
    std::unique_ptr<Else> block_else;
    std::unique_ptr<Lastly> block_lastly;
    Try(lexer::Token const &kwd, std::unique_ptr<Statement> body_try, std::vector<std::unique_ptr<Except>> handlers,
        std::unique_ptr<Else> block_else, std::unique_ptr<Lastly> block_lastly)
      : Statement(kwd), body_try(std::move(body_try)),
        handlers(std::move(handlers)),
        block_else(std::move(block_else)),
        block_lastly(std::move(block_lastly)) {}
    _visit;
  };

  struct Raise: Statement {
    std::unique_ptr<Expression> value;
    Raise(lexer::Token const &kwd, std::unique_ptr<Expression> value)
      : Statement(kwd), value(std::move(value)) {}
    _visit;
  };

  struct Class: Statement {
    std::unique_ptr<Expression> decl;
    std::unique_ptr<Statement> body;
    Class(lexer::Token const &kwd, std::unique_ptr<Expression> decl,
          std::unique_ptr<Statement> body)
      : Statement(kwd), decl(std::move(decl)), body(std::move(body)) {}
    _visit;
  };

  struct Fn: Expression {
    std::unique_ptr<Expression> decl;
    std::unique_ptr<Statement> body;
    Fn(lexer::Token const &kwd, std::unique_ptr<Expression> decl,
       std::unique_ptr<Statement> body)
      : Expression(kwd), decl(std::move(decl)), body(std::move(body)) {}
    _visit;
  };

  struct Import: Statement {
    std::unique_ptr<Expression> module;
    Import(lexer::Token const &kwd, std::unique_ptr<Expression> module)
      : Statement(kwd), module(std::move(module)) {}
    _visit;
  };

  struct From: Statement {
    std::unique_ptr<Expression> package;
    std::unique_ptr<Import> module;
    From(lexer::Token const &kwd, std::unique_ptr<Expression> package,
         std::unique_ptr<Import> module)
      : Statement(kwd), package(std::move(package)), module(std::move(module)) {
    }
    _visit;
  };

  struct If: Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> block_if;
    std::unique_ptr<Else> block_else;
    If(lexer::Token const &kwd, std::unique_ptr<Expression> cond,
       std::unique_ptr<Statement> block_if, std::unique_ptr<Else> block_else)
      : Statement(kwd), condition(std::move(cond)),
        block_if(std::move(block_if)), block_else(std::move(block_else)) {}
    _visit;
  };

  struct While: Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    While(lexer::Token const &kwd, std::unique_ptr<Expression> cond,
          std::unique_ptr<Statement> body)
      : Statement(kwd), condition(std::move(cond)), body(std::move(body)) {}
    _visit;
  };

  struct For: Statement {
    std::unique_ptr<Expression> bindings;
    std::unique_ptr<Statement> body;
    For(lexer::Token const &kwd, std::unique_ptr<Expression> bindings,
        std::unique_ptr<Statement> body)
      : Statement(kwd), bindings(std::move(bindings)), body(std::move(body)) {}
    _visit;
  };

  struct Return: Statement {
    std::unique_ptr<Expression> value;
    Return(lexer::Token const &kwd, std::unique_ptr<Expression> value)
      : Statement(kwd), value(std::move(value)) {}
    _visit;
  };

  struct With: Statement {
    std::unique_ptr<Expression> contexts;
    std::unique_ptr<Statement> block;
    With(lexer::Token const &kwd, std::unique_ptr<Expression> ctx,
         std::unique_ptr<Statement> block)
      : Statement(kwd), contexts(std::move(ctx)), block(std::move(block)) {}
    _visit;
  };

  struct Assert: Statement {
    std::unique_ptr<Expression> condition;
    Assert(lexer::Token const &kwd, std::unique_ptr<Expression> cond)
      : Statement(kwd), condition(std::move(cond)) {}
    _visit;
  };

  struct NonLocal: Statement {
    std::unique_ptr<Expression> identifiers;
    NonLocal(lexer::Token const &kwd, std::unique_ptr<Expression> ids)
      : Statement(kwd), identifiers(std::move(ids)) {}
    _visit;
  };

  struct Global: Statement {
    std::unique_ptr<Expression> identifiers;
    Global(lexer::Token const &kwd, std::unique_ptr<Expression> ids)
      : Statement(kwd), identifiers(std::move(ids)) {}
    _visit;
  };

  struct Del: Statement {
    std::unique_ptr<Expression> identifiers;
    Del(lexer::Token const &kwd, std::unique_ptr<Expression> ids)
      : Statement(kwd), identifiers(std::move(ids)) {}
    _visit;
  };

  struct Assignment: Statement {
    std::unique_ptr<Expression> store, value;
    Assignment(lexer::Token const &op, std::unique_ptr<Expression> store,
               std::unique_ptr<Expression> value)
      : Statement(op), store(std::move(store)), value(std::move(value)) {}
  };

  struct Container: Expression {
    enum class Type {
#define TK_INCLUDE TK_CONTAINERS
#define TK_MACRO(Name, _) Name,
#include "token_macro.hpp"
    } type;
    std::unique_ptr<Expression> entries;
    Container(lexer::Token const &op, Type type,
              std::unique_ptr<Expression> entries)
      : Expression(op), type(type), entries(std::move(entries)) {}
    _visit;
  };

  const char *to_string(Container::Type type);

#define INH_ASSIGN(_Name)                                                      \
  struct _Name: Assignment {                                                   \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> store,           \
          std::unique_ptr<Expression> value)                                   \
      : Assignment(op, std::move(store), std::move(value)) {}                  \
    _visit;                                                                    \
  }

#define INH_BINARY_visit(_Name)                                                \
  struct _Name: BinaryExpr {                                                   \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> left,            \
          std::unique_ptr<Expression> right)                                   \
      : BinaryExpr(op, std::move(left), std::move(right)) {}                   \
    _visit;                                                                    \
  }

#define INH_UNARY_visit(_Name)                                                 \
  struct _Name: UnaryExpr {                                                    \
    _Name(lexer::Token const &op, std::unique_ptr<Expression> operand)         \
      : UnaryExpr(op, std::move(operand)) {}                                   \
    _visit;                                                                    \
  }

#define INH_LITERAL_visit(_Name)                                               \
  struct _Name: Expression {                                                   \
    _Name(lexer::Token const &op): Expression(op) {}                           \
    _visit;                                                                    \
  }

#define TK_MACRO(Name, _) INH_ASSIGN(Name);
#define TK_INCLUDE TK_ASSIGN
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_LITERAL_visit(Name);
#define TK_INCLUDE (TK_PRINUMBER | TK_PRIMARY | TK_KEYLITERAL)
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_BINARY_visit(Name);
#define TK_INCLUDE TK_PRIBINARY
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_BINARY_visit(Name);
#define TK_INCLUDE (TK_KEYBINARY | TK_OPALL)
#include "token_macro.hpp"

#define TK_MACRO(Name, _) INH_UNARY_visit(Name);
#define TK_INCLUDE TK_KEYUNARY
#include "token_macro.hpp"

#undef INH_LITERAL_visit
#undef INH_ASSIGN
#undef INH_UNARY_visit
#undef INH_BINARY_visit
#undef _node_visit
#undef _visit

#pragma GCC diagnostic pop
} // namespace rattle::parser::nodes

