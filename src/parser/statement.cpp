#include <rattle/parser/error.hpp>
#include <rattle/parser/parser.hpp>
#include <rattle/token/token.hpp>
#include <rattle/tree/nodes.hpp>

namespace rattle::parser::internal {
  token::Token Parser::parse_eos() noexcept {
    auto filter = base.with(State::EOS);
    switch (base.iskind()) {
    case token::Kind::Eot:
    case token::Kind::Newline:
    case token::Kind::Semicolon:
      return base.eat(); // Own the EOS marker
    default:
      base.report(error::Kind::expected_eos_marker);
      return base.peek(); // Might be anything, copy and let
      // the parser figure out what it is.
    }
  }

  Scoped<tree::Stmt> Parser::parse_statement() noexcept {
    for (;;) {
      switch (base.iskind()) {
      case token::Kind::Continue: {
        auto kw = base.eat();
        parse_eos();
        return make<tree::stmt::TkExpr>(kw, nullptr);
      }
      case token::Kind::Break: {
        auto kw = base.eat();
        parse_eos();
        return make<tree::stmt::TkExpr>(kw, nullptr);
      }
      case token::Kind::Return: {
        auto kw = base.eat();
        auto value = parse_expression();
        parse_eos();
        return make<tree::stmt::TkExpr>(kw, std::move(value));
      }
      case token::Kind::Nonlocal: {
        auto kw = base.eat();
        auto names = parse_expression();
        parse_eos();
        return make<tree::stmt::TkExpr>(kw, std::move(names));
      }
      case token::Kind::Global: {
        auto kw = base.eat();
        auto names = parse_expression();
        parse_eos();
        return make<tree::stmt::TkExpr>(kw, std::move(names));
      }
      case token::Kind::OpenBrace:
        return parse_block();
      case token::Kind::Def:
        return parse_def();
      case token::Kind::Class:
        return parse_class();
      case token::Kind::If:
        return parse_if();
      case token::Kind::While:
        return parse_while();
      case token::Kind::For:
        return parse_for();
      case token::Kind::CloseBrace:
        if (scopes.in_brace()) {
          return nullptr; // Let caller consume closing token
        } else {
          base.report(error::Kind::dangling_brace, base.eat());
          break;
        }
      case token::Kind::CloseParen:
        if (scopes.in_paren()) {
          return nullptr; // Let caller consume closing token
        } else {
          base.report(error::Kind::dangling_paren, base.eat());
          break;
        }
      case token::Kind::CloseBracket:
        if (scopes.in_bracket()) {
          return nullptr; // Let caller consume closing token
        } else {
          base.report(error::Kind::dangling_bracket, base.eat());
          break;
        }
      default:
        if (auto stmt = parse_assignment()) {
          return stmt;
        }
        base.report(error::Kind::unexpected_token, base.eat());
      }
    }
  }
} // namespace rattle::parser::internal

