#include "rattle/rattle.hpp"
#include <cassert>
#include <rattle/parser/parser.hpp>
#include <rattle/token/token.hpp>
#include <rattle/tree/nodes.hpp>

namespace rattle::parser::internal {
  Scoped<tree::Stmt> Parser::parse_stmt() noexcept {
    for (;;) {
      auto stmt = parse_statement();
      if (stmt) {
        return stmt;
      }
      switch (cursor.merge()) {
      case rattle_merge_kind2(Marker, CloseBrace):
        assert(not scopes.in_scope());
        cursor.report(error::Kind::dangling_brace, cursor.eat());
        break;
      case rattle_merge_kind2(Marker, CloseParen):
        assert(not scopes.in_paren());
        cursor.report(error::Kind::dangling_paren, cursor.eat());
        break;
      case rattle_merge_kind2(Marker, CloseBracket):
        assert(not scopes.in_bracket());
        cursor.report(error::Kind::dangling_bracket, cursor.eat());
        break;
      case rattle_merge_kind(token::kinds::Token::Eot, 0): {
        assert(cursor.empty());
        if (not blocks.empty()) {
          cursor.report(error::Kind::unterminated_brace, blocks.top()->at);
          return leave_scope(cursor.eat());
        } else {
          return nullptr;
        }
      }
      default:
        unreachable();
      }
    }
  }

  Scoped<tree::stmt::Event> Parser::parse_block() noexcept {
    assert(cursor.iskind(token::kinds::Marker::OpenBrace));
    auto lbrace = cursor.eat();
    // We don't know where this block ends so both the end and start
    // share the same token until the closing one is found or Eot is set.
    auto block_enter =
      make<tree::stmt::Event>(tree::stmt::Event::Kind::ScopeBegin, lbrace);
    // I need to know the info about entry to report improper exits
    auto block_exit =
      make<tree::stmt::Event>(tree::stmt::Event::Kind::ScopeBegin, lbrace);
    // Save the block exit event currently holding entry info to aid error reporting
    // Give the stack ownership of the pointer.
    enter_scope(block_exit.release());
    return block_enter;
  }

  Scoped<tree::stmt::TkExprStmt> Parser::parse_tkexprblock() noexcept {
    auto tk = cursor.eat();
    auto expr = parse_expression();
    Scoped<tree::Stmt> block;
    if (cursor.iskind(token::kinds::Marker::OpenBrace)) {
      block = parse_block();
    }
    return make<tree::stmt::TkExprStmt>(tk, std::move(expr), std::move(block));
  }

  Scoped<tree::Stmt> Parser::parse_assignment() noexcept {
    auto left = parse_expression();
    if (cursor.iskind(token::kinds::Token::Assignment)) {
      auto op = cursor.eat();
      auto right = parse_expression();
      return make<tree::stmt::Assignment>(
        op, std::move(left), std::move(right));
    } else if (left) {
      parse_eos();
      return make<tree::stmt::ExprStmt>(std::move(left));
    } else {
      return nullptr;
    }
  }
} // namespace rattle::parser::internal

