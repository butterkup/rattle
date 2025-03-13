#include <cassert>
#include <rattle/ast/node_kinds.hpp>
#include <rattle/ast/nodes.hpp>
#include <rattle/rattle.hpp>
#include <rattle/syntax/analyzer.hpp>
#include <string_view>
#include <vector>

namespace rattle::analyzer::syntax {
  void StatementAnalyzer::visit(tree::stmt::Assignment &stmt) noexcept {
    Result slot{}, value{};
    if (stmt.left) {
      slot = expr_analyzer.analyze<Flags::ListComponentsAssignable>(*stmt.left);
      if (not slot.flags.test(Flags::Assignable)) {
        /* REPORT ERROR */
      }
    } else {
      /* REPORT ERROR */
    }
    if (stmt.right) {
      value = expr_analyzer.analyze(*stmt.right);
    } else {
      /* REPORT ERROR */
    }
    switch (stmt.op.kind) {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(AssignKind, _)                                   \
  case token::Kind::AssignKind:                                                \
    node =                                                                     \
      reactor.make<ast::stmt::Assignment>(ast::kinds::Assignment::AssignKind,  \
        std::move(slot.expr), std::move(value.expr));
#include <rattle/ast/require_pp/assignment.h>
#include <rattle/ast/require_pp/undefine.h>
    default:
      unreachable();
    }
  }

  void StatementAnalyzer::visit(tree::stmt::Block &stmt) noexcept {
    std::vector<Scoped<ast::Stmt>> stmts;
    for (auto &stmt : stmt.statements) {
      stmts.push_back(analyze(*stmt));
    }
    node = reactor.make<ast::stmt::Block>(std::move(stmts), &stmt);
  }

  void StatementAnalyzer::visit(tree::stmt::If &stmt) noexcept {
    Result cond;
    Scoped<ast::Stmt> if_body, else_body;
    if (stmt.if_.cond) {
      cond = expr_analyzer.analyze(*stmt.if_.cond);
    } else {
      /* REPORT ERROR */
    }
    if (stmt.if_.body) {
      if_body = analyze(*stmt.if_.body);
    } else {
      /* REPORT ERROR */
    }
    if (stmt.else_.has_value()) {
      if (stmt.else_.value().body) {
        else_body = analyze(*stmt.else_.value().body);
      } else {
        /* REPORT ERROR */
      }
    }
    node = reactor.make<ast::stmt::If>(
      std::move(cond.expr), std::move(if_body), std::move(else_body), &stmt);
  }

  void StatementAnalyzer::visit(tree::stmt::ExprStmt &stmt) noexcept {
    assert(stmt.expr);
    node = reactor.make<ast::stmt::ExprStmt>(
      expr_analyzer.analyze(*stmt.expr).expr, &stmt);
  }

  void StatementAnalyzer::visit(tree::stmt::TkExpr &stmt) noexcept {
    Result expr;
    switch (stmt.tk.kind) {
    case token::Kind::Continue:
      if (stmt.expr) {
        /* REPORT ERROR */
      }
      node = reactor.make<ast::stmt::LoopControl>(
        ast::kinds::LoopControl::Continue, &stmt);
      break;
    case token::Kind::Break:
      if (stmt.expr) {
        /* REPORT ERROR */
      }
      node = reactor.make<ast::stmt::LoopControl>(
        ast::kinds::LoopControl::Break, &stmt);
      break;
    case token::Kind::Return:
      node = reactor.make<ast::stmt::Command>(ast::kinds::Command::Return,
        (stmt.expr ? expr_analyzer.analyze(*stmt.expr).expr : nullptr), &stmt);
      break;
    case token::Kind::Nonlocal:
      expr = expr_analyzer.analyze<Flags::ListOfIDsOnly>(*stmt.expr);
      if (expr.flags.test(Flags::OnlyIDs)) {
        node = reactor.make<ast::stmt::Command>(
          ast::kinds::Command::Nonlocal, std::move(expr.expr), &stmt);
      } else {
        /* REPORT ERROR */
      }
      break;
    case token::Kind::Global:
      expr = expr_analyzer.analyze<Flags::ListOfIDsOnly>(*stmt.expr);
      if (expr.flags.test(Flags::OnlyIDs)) {
        node = reactor.make<ast::stmt::Command>(
          ast::kinds::Command::Global, std::move(expr.expr), &stmt);
      } else {
        /* REPORT ERROR */
      }
      break;
    default:
      unreachable();
    }
  }

  void StatementAnalyzer::visit(tree::stmt::TkExprBlock &stmt) noexcept {
    Scoped<ast::Stmt> body;
    if (not stmt.expr) {
      /* REPORT ERROR */
    }
    if (stmt.block) {
      body = analyze(*stmt.block);
    } else {
      /* REPORT ERROR */
    }
    switch (stmt.tk.kind) {
    case token::Kind::For: {
      Result expr = stmt.expr ? expr_analyzer.analyze<Flags::LeftBindable1stIn>(
                                  *stmt.expr) :
                                Result{};
      if (expr.flags.test(Flags::In)) {
        auto &tmp = rattle_cast<ast::expr::BinaryExpr>(*expr.expr);
        node = reactor.make<ast::stmt::For>(
          std::move(tmp.left), std::move(tmp.right), std::move(body), &stmt);
      } else {
        /* REPORT ERROR */
      }
      break;
    }
    case token::Kind::While: {
      Result expr = stmt.expr ? expr_analyzer.analyze(*stmt.expr) : Result{};
      node = reactor.make<ast::stmt::While>(
        std::move(expr.expr), std::move(body), &stmt);
      break;
    }
    case token::Kind::Def: {
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
      } else {
        /* REPORT ERROR */
      }
      break;
    }
    case token::Kind::Class: {
      Result expr = stmt.expr ? expr_analyzer.analyze(*stmt.expr) : Result{};
      if (expr.flags.test(Flags::LiteralID)) {
        std::string_view name =
          rattle_cast<ast::expr::Literal>(*expr.expr).value;
        node = reactor.make<ast::stmt::Class>(name, std::move(body), &stmt);
      } else {
        /* REPORT ERROR */
      }
      break;
    }
    default:
      unreachable();
    }
  }
} // namespace rattle::analyzer::syntax

