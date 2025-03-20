#pragma once

#include "api.hpp"
#include "cursor.hpp"
#include "error.hpp"
#include "utility.hpp"
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>
#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>
#include <utility>

namespace rattle::parser {
  namespace internal {
    using utility::Scoped;

    // Simple LL(1) parser for `rattle`, default impl
    struct Parser {
      Parser(ILexer &lexer, IReactor &reactor)
        : cursor{lexer, reactor}, scopes{}, reactor{reactor}, blocks{reactor} {}
      Scoped<tree::Stmt> parse_stmt() noexcept;
      bool empty() const noexcept { return cursor.empty() and blocks.empty(); }
      void drain() noexcept { return cursor.drain_program(); }

      // A way to construct parse tree nodes without fragmenting memory,
      // assuming the reactor is taking from an arena, whatever they get memory
      // I don't care, just give me some, inplace construct and give ownership
      // to scoped to call destructor when the pointer goes out of scope.
      // NOTE: We cannot use unique_ptr since it calls delete but we don't own
      // the memory, reactor does. Since all objects are either in use or dropped
      // then we can simply call dtor and later the arena will bulk free all memory.
      template <class Type, class... Args>
      Scoped<Type> make(Args &&...args) noexcept {
        // Allocate some piece of memory to hold `Type` object
        Type *slab = static_cast<Type *>(reactor.allocate(sizeof(Type)));
        if (slab == nullptr) {
          // No memory from reactor, let the reactor know what to do.
          // NOTE: `OnError::Abort` is the most likely strategy here since before allocate
          // returned, the reactor knew of the lack of memory. If not abort, empty
          // `Scoped`s will be returned which will discard the args and much more.
          // XXX: Empty `Scoped`s will indicate to the analyzer that no more can be produced
          // or atleast an error occurred, method `empty` will determine the truth.
          cursor.report(error::Kind::reactor_out_of_memory);
        } else {
          // Construct `Type` on allocated memory
          new (slab) Type(std::forward<Args>(args)...);
        }
        // Let `Scoped` handle destruction and ownership of the object if owning
        return Scoped{slab};
      }

      Cursor cursor;
      Scopes scopes;

    private:
      IReactor &reactor;
      Stack<tree::stmt::Event *, 10, IReactor> blocks;

      inline void enter_scope(tree::stmt::Event *blockinfo) {
        scopes.enter_scope();
        blocks.push(blockinfo);
      }
      inline Scoped<tree::stmt::Event> leave_scope(token::Token const &rbrace) {
        assert(scopes.in_scope());
        assert(not blocks.empty());
        auto end_block = blocks.top();
        *end_block = {tree::stmt::Event::Kind::ScopeEnd, rbrace};
        blocks.pop(); // Take ownership
        scopes.leave_scope();
        return end_block;
      }
      Scoped<tree::stmt::Event> parse_block() noexcept;
      Scoped<tree::stmt::TkExprStmt> parse_tkexprblock() noexcept;
      Scoped<tree::Expr> parse_expression() noexcept;
      Scoped<tree::Stmt> parse_statement() noexcept;
      Scoped<tree::Stmt> parse_assignment() noexcept;
      token::Token parse_eos() noexcept;
    };
  } // namespace internal
  // Wraps internal parser conforming with the `IParser` API.
  struct Parser: IParser {
    Parser(ILexer &lexer, IReactor &reactor): parser{lexer, reactor} {}

    [[nodiscard]] utility::Scoped<tree::Stmt> parse() noexcept override {
      return parser.parse_stmt();
    }
    bool empty() const noexcept override { return parser.empty(); }
    void drain() noexcept override { parser.drain(); }

  private:
    internal::Parser parser;
  };
} // namespace rattle::parser

