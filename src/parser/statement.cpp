#include <cassert>
#include <rattle/parser/error.hpp>
#include <rattle/parser/parser.hpp>
#include <rattle/token/token.hpp>
#include <rattle/tree/nodes.hpp>

namespace rattle::parser::internal {
  token::Token Parser::parse_eos() noexcept {
    auto filter = cursor.with(State::EOS);
    switch (cursor.merge()) {
    case rattle_merge_kind(token::kinds::Token::Eot, 0):
    case rattle_merge_kind2(Marker, Newline):
    case rattle_merge_kind2(Marker, Semicolon):
      return cursor.eat(); // Own the EOS marker
    default:
      cursor.report(error::Kind::expected_eos_marker);
      return cursor.peek(); // Might be anything, copy and let
      // the parser figure out what it is.
    }
  }

  Scoped<tree::Stmt> Parser::parse_statement() noexcept {
    for (;;) {
      switch (cursor.merge()) {
      case rattle_merge_kind2(Identifier, Continue): {
        auto kw = cursor.eat();
        parse_eos();
        return make<tree::stmt::Event>(tree::stmt::Event::Kind::Continue, kw);
      }
      case rattle_merge_kind2(Identifier, Break): {
        auto kw = cursor.eat();
        parse_eos();
        return make<tree::stmt::Event>(tree::stmt::Event::Kind::Break, kw);
      }
      case rattle_merge_kind2(Identifier, Def):
      case rattle_merge_kind2(Identifier, Class):
      case rattle_merge_kind2(Identifier, While):
      case rattle_merge_kind2(Identifier, For):
      case rattle_merge_kind2(Identifier, If):
        return parse_tkexprblock();
      case rattle_merge_kind2(Identifier, Nonlocal):
      case rattle_merge_kind2(Identifier, Global):
      case rattle_merge_kind2(Identifier, Return): {
        auto kw = cursor.eat();
        auto value = parse_expression();
        parse_eos();
        return make<tree::stmt::TkExpr>(kw, std::move(value));
      }
      case rattle_merge_kind2(Identifier, Else): {
        auto kw = cursor.eat();
        Scoped<tree::Stmt> body;
        // Allow only `else { }` or `else if cond { }`
        if (cursor.iskind(rattle_merge_kind2(Identifier, If)) or
            cursor.iskind(rattle_merge_kind2(Marker, OpenBrace))) {
          body = parse_statement();
        }
        return make<tree::stmt::TkExprStmt>(kw, nullptr, std::move(body));
      }
      case rattle_merge_kind2(Marker, OpenBrace):
        return parse_block();
      case rattle_merge_kind2(Marker, CloseBrace):
        if (scopes.in_scope()) {
          return leave_scope(cursor.eat());
        } else {
          cursor.report(error::Kind::dangling_brace, cursor.eat());
          break;
        }
      case rattle_merge_kind2(Marker, CloseParen):
        if (scopes.in_paren()) {
          return nullptr; // Let caller consume closing token
        } else {
          cursor.report(error::Kind::dangling_paren, cursor.eat());
          break;
        }
      case rattle_merge_kind2(Marker, CloseBracket):
        if (scopes.in_bracket()) {
          return nullptr; // Let caller consume closing token
        } else {
          cursor.report(error::Kind::dangling_bracket, cursor.eat());
          break;
        }
      case rattle_merge_kind(token::kinds::Token::Eot, 0):
        return nullptr;
      default:
        if (auto stmt = parse_assignment()) {
          return stmt;
        }
        cursor.report(error::Kind::unexpected_token, cursor.eat());
      }
    }
  }
} // namespace rattle::parser::internal

