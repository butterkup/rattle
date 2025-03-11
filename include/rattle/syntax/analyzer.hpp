#pragma once

#include "api.h"
#include <rattle/analyzer.hpp>
#include <rattle/parser.hpp>
#include <rattle/rattle.hpp>
#include <rattle/tree/api.hpp>
#include <type_traits>

namespace rattle::analyzer {
  namespace syntax {
    using utility::Scoped;

    struct Reactor {
      Reactor(IReactor &inner): inner{inner} {}
      IReactor &inner;
      template <typename Type, typename... Args>
        requires std::is_constructible_v<Type, Args...>
      Scoped<Type> make(Args &&...args) noexcept(
        std::is_nothrow_constructible_v<Type, Args...>) {
        void *slab = inner.allocate(sizeof(Type));
        if (slab == nullptr) {
          /* REPORT ERROR */
          unreachable();
        } else {
          new (slab) Type(std::forward<Args>(args)...);
        }
        return static_cast<Type *>(slab);
      }
    };

    struct Flags {
      enum {
        NORMAL = 0x000fffff, // Lower flags hold expr info
        // Upper flags are for callers to pass constaraints on the expression
        CONSTRAINTS = 0xfff00000,
        None = 0, // Nothing to say about the expression
        // Normal flags
        In = 1 << 1,         // To verify `For` statement syntax
        Signature = 1 << 2,  // class and def statement signatures
        Assignable = 1 << 3, // Is the expression assignable
        LiteralID = 1 << 4,  // Is the expression an identifier
        Comma = 1 << 5,   // Is the expression a pair of comma separated exprs
        If = 1 << 6,      // To verify `if-else` ternary expression
        Binding = 1 << 7, // Is the expression bindable
        OnlyIDs = 1 << 8, // Is the expression only composed of IDs
        // Constraint flags
        ListComponentsAssignable = 1 << 20, // Assert left and right
                                            // of comma is assignable
        PreferBinding = 1 << 21,            // Prefer binding over normal nodes
        LeftBindable1stIn = 1 << 22,        // The first `in` expression
        // should have its left value bindable
        ListOfIDsOnly = 1 << 23, // Assert the Comma has ID for left
                                 // and right argument
      };

      Flags(int f = None): flags{f} {}
      bool test(int flags) const noexcept {
        return (this->flags & flags) == flags;
      }
      bool test_any(int flags) const noexcept {
        return (this->flags & flags) != 0;
      }
      void set(int flags_) noexcept { flags |= flags_; }
      void unset(int flags_) noexcept { flags &= ~flags_; }
      int get() const noexcept { return flags; }

    private:
      int flags;
    };

    struct Result {
      Scoped<ast::Expr> expr;
      Flags flags;
    };

#define create_visit(Node) void visit(Node &) noexcept override;

    class ExpressionAnalyzer: private tree::visitor::Expression {
#define rattle_pp_token_macro(Node, _) create_visit(tree::expr::Node)
#define rattle_undef_token_macro
#include <rattle/tree/require_pp/expression.h>
#include <rattle/tree/require_pp/undefine.h>

      Flags flags;
      int level;
      Reactor &reactor;
      Scoped<ast::Expr> node;

    public:
      ExpressionAnalyzer(Reactor &reactor)
        : tree::visitor::Expression{}, flags{}, level{-1}, reactor{reactor},
          node{nullptr} {}

      template <unsigned ConstraintFlags = Flags::None>
      Result analyze(tree::Expr &expr) noexcept {
        level++;
        Flags temp = flags;
        flags = Flags{(ConstraintFlags | temp.get()) & Flags::CONSTRAINTS};
        expr.visit(*this);
        flags = temp;
        level--;
        return {std::move(node), flags};
      }
    };

    class StatementAnalyzer: private tree::visitor::Statement {
#define rattle_pp_token_macro(Node, _) create_visit(tree::stmt::Node)
#define rattle_undef_token_macro
#include <rattle/tree/require_pp/statement.h>
#include <rattle/tree/require_pp/undefine.h>
      Reactor &reactor;
      Scoped<ast::Stmt> node;
      ExpressionAnalyzer &expr_analyzer;

    public:
      StatementAnalyzer(Reactor &reactor, ExpressionAnalyzer &expr_analyzer)
        : tree::visitor::Statement{}, reactor{reactor}, node{nullptr},
          expr_analyzer{expr_analyzer} {}
      Scoped<ast::Stmt> analyze(tree::Stmt &stmt) noexcept {
        stmt.visit(*this);
        return std::move(node);
      }
    };

#undef create_visit

    class Analyzer {
      IParser &parser;
      Reactor reactor;
      ExpressionAnalyzer expr_analyzer;
      StatementAnalyzer stmt_analyzer;

    public:
      Analyzer(IReactor &ireactor, IParser &parser)
        : parser{parser}, reactor{ireactor}, expr_analyzer{reactor},
          stmt_analyzer{reactor, expr_analyzer} {}

      bool empty() const noexcept { return parser.empty(); }
      void drain() noexcept { parser.drain(); }

      Scoped<ast::Stmt> analyze() noexcept {
        Scoped<tree::Stmt> stmt = parser.parse();
        // We don't want SEGFAULTS caused by null deref.
        return stmt ? stmt_analyzer.analyze(*stmt) : nullptr;
      }
    };
  } // namespace syntax

  class SyntaxAnalyzer: ISyntaxAnalyzer {
    syntax::Analyzer analyzer;

  public:
    SyntaxAnalyzer(syntax::IReactor &reactor, IParser &parser)
      : analyzer{reactor, parser} {}
    bool empty() const noexcept override { return analyzer.empty(); }
    void drain() noexcept override { analyzer.drain(); }
    utility::Scoped<ast::Stmt> analyze() noexcept override {
      return analyzer.analyze();
    }
  };
} // namespace rattle::analyzer

