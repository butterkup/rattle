#include <cassert>
#include <rattle/ast/node_kinds.hpp>
#include <rattle/ast/nodes.hpp>
#include <rattle/rattle.hpp>
#include <rattle/syntax/analyzer.hpp>
#include <rattle/token/token.hpp>
#include <rattle/tree/nodes.hpp>

namespace rattle::analyzer::syntax {
  void ExpressionAnalyzer::visit(tree::expr::Literal &expr) noexcept {
    if (expr.value.kind == token::Kind::Identifier) {
      flags.set(Flags::Assignable | Flags::LiteralID | Flags::OnlyIDs);
      if (flags.test(Flags::PreferBinding)) {
        node = reactor.make<ast::expr::Binding>(
          ast::kinds::Binding::Name, expr.value.lexeme, &expr);
        flags.set(Flags::Binding);
        return;
      }
    }
    switch (expr.value.kind) {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(Type, _)                                         \
  case token::Kind::Type:                                                      \
    node = reactor.make<ast::expr::Literal>(                                   \
      ast::kinds::Literal::Type, expr.value.lexeme, &expr);                    \
    break;
#include <rattle/tree/require_pp/literal.h>
#include <rattle/tree/require_pp/undefine.h>
    case token::Kind::Error:
      node = nullptr;
      break;
    default:
      unreachable();
    }
  }

  void ExpressionAnalyzer::visit(tree::expr::UnaryExpr &expr) noexcept {
    Result temp{};
    if (expr.operand) {
      temp = analyze(*expr.operand);
    } else {
      /* REPORT ERROR */
    }
    switch (expr.op.kind) {
    case token::Kind::Plus:
      node = reactor.make<ast::expr::UnaryExpr>(
        ast::kinds::UnaryExpr::Posify, std::move(temp.expr), &expr);
    case token::Kind::Star:
      if (flags.test(Flags::PreferBinding)) {
        if (not temp.flags.test(Flags::LiteralID)) {
          /* REPORT ERROR */
        }
        auto &tmp = rattle_cast<ast::expr::Literal>(*temp.expr);
        node = reactor.make<ast::expr::Binding>(
          ast::kinds::Binding::Capture, tmp.value, &expr);
        flags.set(Flags::Binding);
      } else {
        node = reactor.make<ast::expr::UnaryExpr>(
          ast::kinds::UnaryExpr::Spread, std::move(temp.expr), &expr);
      }
    case token::Kind::Minus:
      node = reactor.make<ast::expr::UnaryExpr>(
        ast::kinds::UnaryExpr::Negate, std::move(temp.expr), &expr);
    case token::Kind::Not:
      node = reactor.make<ast::expr::UnaryExpr>(
        ast::kinds::UnaryExpr::LogicNOT, std::move(temp.expr), &expr);
    default:
      unreachable();
    }
  }

  void ExpressionAnalyzer::visit(tree::expr::BinaryExpr &expr) noexcept {
    Result left{}, right{};
    if (expr.left) {
      left = analyze(*expr.left);
    } else {
      /* REPORT ERROR */
    }
    if (expr.right) {
      right = analyze(*expr.right);
    } else if (expr.op.kind != token::Kind::Comma) {
      /* REPORT ERROR */
    }
    switch (expr.op.kind) {
    case token::Kind::Comma:
      if (flags.test(Flags::ListComponentsAssignable)) {
        if (not left.flags.test(Flags::Assignable)) {
          /* REPORT ERROR */
        }
        if (right.expr and not right.flags.test(Flags::Assignable)) {
          /* REPORT ERROR */
        }
      }
      if (flags.test(Flags::PreferBinding)) {
        if (not left.flags.test(Flags::Binding)) {
          /* REPORT ERROR */
        }
        if (right.expr and not right.flags.test(Flags::Binding)) {
          /* REPORT ERROR */
        }
      }
      if (flags.test(Flags::ListOfIDsOnly)) {
        if (not left.flags.test(Flags::OnlyIDs)) {
          /* REPORT ERROR */
        }
        if (not right.flags.test(Flags::OnlyIDs)) {
          /* REPORT ERROR */
        }
      }
      // Activate the bits iff both the left and right have them on
      flags.set(left.flags.get() &
                (right.expr ? right.flags.get() : Flags::Assignable) &
                Flags::Assignable);
      flags.set(left.flags.get() &
                (right.expr ? right.flags.get() : Flags::Binding) &
                Flags::OnlyIDs);
      flags.set(left.flags.get() &
                (right.expr ? right.flags.get() : Flags::OnlyIDs) &
                Flags::OnlyIDs);
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Comma,
        std::move(left.expr), std::move(right.expr), &expr);
      flags.set(Flags::Comma);
      break;
    case token::Kind::If:
      node = reactor.make<ast::expr::BinaryExpr>(
        static_cast<ast::kinds::BinaryExpr>(-1), std::move(left.expr),
        std::move(right.expr), &expr);
      flags.set(Flags::If);
    case token::Kind::Plus:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Add,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::Minus:
      node =
        reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Subtract,
          std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::Star:
      node =
        reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Multiply,
          std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::Slash:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Divide,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::Or:
      node =
        reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::LogicOR,
          std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::And:
      node =
        reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::LogicAND,
          std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::EqualEqual:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::CmpEQ,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::NotEqual:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::CmpNE,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::LessThan:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::CmpLT,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::LessEqual:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::CmpLE,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::GreaterThan:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::CmpGT,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::GreaterEqual:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::CmpGE,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::In:
      if (flags.test(Flags::LeftBindable1stIn) and level == 0 and
          not left.flags.test(Flags::Binding) and left.expr) {
        /* REPORT ERROR */
      }
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::In,
        std::move(left.expr), std::move(right.expr), &expr);
      flags.set(Flags::In);
      break;
    case token::Kind::Is:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Is,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::OpenParen:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Call,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::OpenBracket:
      node =
        reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Subscript,
          std::move(left.expr), std::move(right.expr), &expr);
      flags.set(Flags::Assignable);
      break;
    case token::Kind::Dot:
      if (not right.flags.test(Flags::LiteralID)) {
        /* REPORT ERROR */
      }
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Dot,
        std::move(left.expr), std::move(right.expr), &expr);
      flags.set(Flags::Assignable);
      break;
    default:
      unreachable();
    }
  }

  void ExpressionAnalyzer::visit(tree::expr::BiExprBiTk &expr) noexcept {
    Result left{}, right{};
    if (expr.expr1) {
      left = analyze(*expr.expr1);
    } else if (expr.tk1.kind != token::Kind::OpenParen) {
      /* REPORT ERROR */
    }
    if (expr.expr2) {
      right = analyze(*expr.expr2);
    } else if (expr.tk1.kind != token::Kind::OpenParen) {
      /* REPORT ERROR */
    }
    switch (expr.tk1.kind) {
    case token::Kind::If: {
      assert(left.flags.test(Flags::If));
      auto &temp = rattle_cast<ast::expr::BinaryExpr>(*left.expr);
      node = reactor.make<ast::expr::TernaryExpr>(
        ast::kinds::TernaryExpr::IfElse, std::move(temp.left),
        std::move(temp.right), std::move(right.expr), &expr);
      break;
    }
    case token::Kind::OpenParen:
      if (left.expr) {
        if (left.flags.test(Flags::LiteralID) and
            right.flags.test(Flags::Binding)) {
          flags.set(Flags::Signature);
        }
        node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Call,
          std::move(left.expr), std::move(right.expr), &expr);
      } else if (right.flags.test(Flags::Comma) or not right.expr) {
        node = reactor.make<ast::expr::UnaryExpr>(
          ast::kinds::UnaryExpr::Tuple, std::move(right.expr), &expr);
      } else {
        if (right.flags.test(Flags::Assignable)) {
          flags.set(Flags::Assignable);
        }
        node = reactor.make<ast::expr::UnaryExpr>(
          ast::kinds::UnaryExpr::Group, std::move(right.expr), &expr);
      }
      break;
    case token::Kind::OpenBracket:
      if (left.expr) {
        node =
          reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::Subscript,
            std::move(left.expr), std::move(right.expr), &expr);
      } else {
        if (not right.expr) {
          /* REPORT ERROR */
        }
        node = reactor.make<ast::expr::UnaryExpr>(
          ast::kinds::UnaryExpr::List, std::move(right.expr), &expr);
      }
      break;
    case token::Kind::Is:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::IsNot,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    case token::Kind::Not:
      node = reactor.make<ast::expr::BinaryExpr>(ast::kinds::BinaryExpr::NotIn,
        std::move(left.expr), std::move(right.expr), &expr);
      break;
    default:
      unreachable();
    }
  }
} // namespace rattle::analyzer::syntax

