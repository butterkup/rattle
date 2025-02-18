#pragma once

#include "api.hpp"
#include "cursor.hpp"
#include "rattle/tree/nodes.hpp"
#include <memory>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>

namespace rattle::parser {
  namespace internal {
    // Simple LL(1) parser for `rattle`, default impl
    struct Parser {
      Parser(ILexer &lexer, IReactor &reactor): base{lexer, reactor} {}

    private:
      Cursor base;
      std::unique_ptr<tree::Block> parse_block();
      std::unique_ptr<tree::Stmt> parse_if();
      std::unique_ptr<tree::Stmt> parse_else();
      std::unique_ptr<tree::Def> parse_def();
      std::unique_ptr<tree::Class> parse_class();
      std::unique_ptr<tree::Expr> parse_expression();
      std::unique_ptr<tree::For> parse_for();
      std::unique_ptr<tree::While> parse_while();
      std::unique_ptr<tree::Stmt> parse_trivial();
    };
  } // namespace internal
  // Wraps internal parser conforming with the `IParser` API.
  struct Parser: IParser {
    Parser(ILexer &lexer, IReactor &reactor): parser{lexer, reactor} {}

  private:
    internal::Parser parser;
  };
} // namespace rattle::parser

