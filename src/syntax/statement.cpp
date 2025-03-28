#include <cassert>
#include <rattle/ast/node_kinds.hpp>
#include <rattle/ast/nodes.hpp>
#include <rattle/rattle.hpp>
#include <rattle/syntax/analyzer.hpp>
#include <rattle/token/token.hpp>
#include <string_view>

namespace rattle::analyzer::syntax {
  void StatementAnalyzer::visit(tree::stmt::Assignment &stmt) noexcept {
    Result slot, value;
    if (stmt.left) {
      slot = expr_analyzer.analyze<Flags::ListComponentsAssignable>(*stmt.left);
      if (not slot.flags.test(Flags::Assignable)) {
        /* REPORT ERROR */
        report("not assignable", slot);
      }
    } else {
      /* REPORT ERROR */
      report("expected an assignable target", stmt.op);
    }
    if (stmt.right) {
      value = expr_analyzer.analyze(*stmt.right);
    } else {
      /* REPORT ERROR */
      report("expected an expression after assignment operator", stmt.op);
    }
    assert(stmt.op.kind == token::kinds::Token::Assignment);
    switch (stmt.op.flags) {
#define rattle_pp_token_macro(AssignKind, _)                                   \
  case token::kinds::Assignment::AssignKind:                                   \
    node =                                                                     \
      reactor.make<ast::stmt::Assignment>(ast::kinds::Assignment::AssignKind,  \
        std::move(slot.expr), std::move(value.expr));                          \
    break;
#include <rattle/ast/require_pp/assignment.h>
#undef rattle_pp_token_macro
    default:
      unreachable();
    }
  }

  void StatementAnalyzer::visit(tree::stmt::ExprStmt &stmt) noexcept {
    assert(stmt.expr);
    node = reactor.make<ast::stmt::ExprStmt>(
      expr_analyzer.analyze(*stmt.expr).expr, &stmt);
  }

  void StatementAnalyzer::visit(tree::stmt::TkExpr &stmt) noexcept {
    Result expr;
    switch (stmt.tk.merge()) {
    case rattle_merge_kind2(Identifier, Return):
      node = reactor.make<ast::stmt::Command>(ast::kinds::Command::Return,
        (stmt.expr ? expr_analyzer.analyze(*stmt.expr).expr : nullptr), &stmt);
      break;
    case rattle_merge_kind2(Identifier, Nonlocal):
      if (stmt.expr) {
        expr = expr_analyzer.analyze<Flags::ListOfIDsOnly>(*stmt.expr);
      } else {
        /* REPORT ERROR */
        report("expected an expression", stmt.tk);
      }
      if (expr.flags.test(Flags::OnlyIDs)) {
        node = reactor.make<ast::stmt::Command>(
          ast::kinds::Command::Nonlocal, std::move(expr.expr), &stmt);
      } else {
        /* REPORT ERROR */
        report("expected an identifier or comma separated list of identifiers",
          expr);
      }
      break;
    case rattle_merge_kind2(Identifier, Global):
      if (stmt.expr) {
        expr = expr_analyzer.analyze<Flags::ListOfIDsOnly>(*stmt.expr);
      } else {
        /* REPORT ERROR */
        report("expected an expression", stmt.tk);
      }
      if (expr.flags.test(Flags::OnlyIDs)) {
        node = reactor.make<ast::stmt::Command>(
          ast::kinds::Command::Global, std::move(expr.expr), &stmt);
      } else {
        /* REPORT ERROR */
        report("expected an identifier or comma separated list of identifiers",
          expr);
      }
      break;
    default:
      unreachable();
    }
  }

  void StatementAnalyzer::visit(tree::stmt::Event &stmt) noexcept {
    switch (stmt.kind) {
    case tree::stmt::Event::Kind::ScopeBegin:
      node =
        reactor.make<ast::stmt::Event>(ast::kinds::Event::ScopeBegin, &stmt);
      break;
    case tree::stmt::Event::Kind::ScopeEnd:
      node = reactor.make<ast::stmt::Event>(ast::kinds::Event::ScopeEnd, &stmt);
      break;
    case tree::stmt::Event::Kind::Continue:
      node = reactor.make<ast::stmt::Event>(ast::kinds::Event::Continue, &stmt);
      break;
    case tree::stmt::Event::Kind::Break:
      node = reactor.make<ast::stmt::Event>(ast::kinds::Event::Break, &stmt);
      break;
    default:
      unreachable();
    }
  }

  void StatementAnalyzer::visit(tree::stmt::TkExprStmt &stmt) noexcept {
    Scoped<ast::Stmt> body;
    if (stmt.tk.merge() != rattle_merge_kind2(Identifier, Else) and
        not stmt.expr) {
      /* REPORT ERROR */
      report("expected an expression after token", stmt.tk);
    }
    if (stmt.block) {
      body = analyze(*stmt.block);
    } else {
      /* REPORT ERROR */
      report("expected block statement for statement", stmt.tk);
    }
    switch (stmt.tk.merge()) {
    case rattle_merge_kind2(Identifier, For): {
      Result expr = stmt.expr ? expr_analyzer.analyze<Flags::LeftBindable1stIn>(
                                  *stmt.expr) :
                                Result{};
      if (expr.flags.test(Flags::In)) {
        auto &tmp = rattle_cast<ast::expr::BinaryExpr>(*expr.expr);
        node = reactor.make<ast::stmt::For>(
          std::move(tmp.left), std::move(tmp.right), std::move(body), &stmt);
      } else if (expr.expr) {
        /* REPORT ERROR */
        report("expected an `in` expression", expr);
      }
      break;
    }
    case rattle_merge_kind2(Identifier, While): {
      Result expr = stmt.expr ? expr_analyzer.analyze(*stmt.expr) : Result{};
      node = reactor.make<ast::stmt::While>(
        std::move(expr.expr), std::move(body), &stmt);
      break;
    }
    case rattle_merge_kind2(Identifier, Def): {
      Result expr = stmt.expr ?
                      expr_analyzer.analyze<Flags::PreferBinding>(*stmt.expr) :
                      Result{};
      if (expr.flags.test(Flags::Signature)) {
        auto &tmp = rattle_cast<ast::expr::BinaryExpr>(*expr.expr);
        std::string_view name = rattle_cast<ast::expr::Binding>(*tmp.left).name;
        Scoped<ast::Expr> params =
          std::move(rattle_cast<ast::expr::UnaryExpr>(*tmp.right).operand);
        node = reactor.make<ast::stmt::Def>(ast::kinds::Def::Function, name,
          std::move(params), std::move(body), &stmt);
      } else if (expr.expr) {
        /* REPORT ERROR */
        report(
          "expected a signature expression like `name(identifier, *capture)`",
          expr);
      }
      break;
    }
    case rattle_merge_kind2(Identifier, Class): {
      Result expr = stmt.expr ? expr_analyzer.analyze(*stmt.expr) : Result{};
      if (expr.flags.test(Flags::LiteralID)) {
        std::string_view name =
          rattle_cast<ast::expr::Literal>(*expr.expr).value;
        node = reactor.make<ast::stmt::Class>(name, std::move(body), &stmt);
      } else if (expr.expr) {
        /* REPORT ERROR */
        report("expected class name", expr);
      }
      break;
    }
    case rattle_merge_kind2(Identifier, If): {
      Result expr = stmt.expr ? expr_analyzer.analyze(*stmt.expr) : Result{};
      node = reactor.make<ast::stmt::If>(
        std::move(expr.expr), std::move(body), &stmt);
      break;
    }
    case rattle_merge_kind2(Identifier, Else): {
      node = reactor.make<ast::stmt::Else>(std::move(body), &stmt);
      break;
    }
    default:
      unreachable();
    }
  }
} // namespace rattle::analyzer::syntax

