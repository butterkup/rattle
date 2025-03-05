#include <cassert>
#include <optional>
#include <rattle/parser/parser.hpp>
#include <rattle/token/token.hpp>
#include <rattle/tree/nodes.hpp>
#include <vector>

namespace rattle::parser::internal {
  Scoped<tree::Stmt> Parser::parse_stmt() noexcept {
    for (;;) {
      auto stmt = parse_statement();
      if (stmt) {
        return stmt;
      }
      switch (base.iskind()) {
      case token::Kind::CloseBrace:
        assert(not scopes.in_brace());
        base.report(error::Kind::dangling_brace, base.eat());
        break;
      case token::Kind::CloseParen:
        assert(not scopes.in_paren());
        base.report(error::Kind::dangling_paren, base.eat());
        break;
      case token::Kind::CloseBracket:
        assert(not scopes.in_bracket());
        base.report(error::Kind::dangling_bracket, base.eat());
        break;
      case token::Kind::Eot:
        return stmt;
      default:
        assert(false);
      }
    }
  }

  Scoped<tree::Block> Parser::parse_block() noexcept {
    assert(base.iskind(token::Kind::OpenBrace));
    auto lbrace = base.eat();
    std::vector<Scoped<tree::Stmt>> stmts;
    auto scope = scopes.enter_brace();
    for (;;) {
      auto stmt = parse_statement();
      if (stmt) {
        stmts.push_back(std::move(stmt));
        continue;
      }
      switch (base.iskind()) {
      case token::Kind::Eot:
        // Malformed block statement, but okay for analysable.
        base.report(error::Kind::unterminated_brace, lbrace);
      case token::Kind::CloseBrace:
        return make<tree::Block>(lbrace, base.eat(), std::move(stmts));
      default:
        assert(false); // Not reacheable!
      }
    }
  }

#define optional_parse_block()                                                 \
  base.iskind(token::Kind::OpenBrace) ? parse_block() : nullptr

  Scoped<tree::TkExprBlock> Parser::parse_for() noexcept {
    assert(base.iskind(token::Kind::For));
    auto kw = base.eat();
    auto binding = parse_expression();
    return make<tree::TkExprBlock>(
      kw, std::move(binding), optional_parse_block());
  }

  Scoped<tree::TkExprBlock> Parser::parse_while() noexcept {
    assert(base.iskind(token::Kind::While));
    auto kw = base.eat();
    auto binding = parse_expression();
    return make<tree::TkExprBlock>(
      kw, std::move(binding), optional_parse_block());
  }

  Scoped<tree::TkExprBlock> Parser::parse_def() noexcept {
    assert(base.iskind(token::Kind::Def));
    auto kw = base.eat();
    auto name_params = parse_expression();
    return make<tree::TkExprBlock>(
      kw, std::move(name_params), optional_parse_block());
  }

  Scoped<tree::TkExprBlock> Parser::parse_class() noexcept {
    assert(base.iskind(token::Kind::Class));
    auto kw = base.eat();
    auto name_bases = parse_expression();
    return make<tree::TkExprBlock>(
      kw, std::move(name_bases), optional_parse_block());
  }

  Scoped<tree::If> Parser::parse_if() noexcept {
    assert(base.iskind(token::Kind::If));
    auto kw = base.eat();
    auto cond = parse_expression();
    auto block = optional_parse_block();
    tree::internal::If if_{kw, std::move(cond), std::move(block)};
    std::optional<tree::internal::Else> else_ = std::nullopt;
    if (base.iskind(token::Kind::Else)) {
      auto kw2 = base.eat();
      auto block2 = optional_parse_block();
      else_.emplace(kw2, std::move(block));
    }
    return make<tree::If>(std::move(if_), std::move(else_));
  }

  Scoped<tree::Stmt> Parser::parse_assignment() noexcept {
    auto left = parse_expression();
    switch (base.iskind()) {
#define rattle_pp_token_macro(kind, _) case token::Kind::kind:
#include <rattle/token/require_pp/assignment.h>
      break;
    default:
      if (left) {
        parse_eos();
        return make<tree::ExprStmt>(std::move(left));
      } else {
        return nullptr;
      }
    }
    auto op = base.eat();
    auto right = parse_expression();
    return make<tree::Assignment>(op, std::move(left), std::move(right));
  }
} // namespace rattle::parser::internal

