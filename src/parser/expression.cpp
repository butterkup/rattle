#include "precedence.hpp"
#include <cassert>
#include <rattle/parser/cursor.hpp>
#include <rattle/parser/error.hpp>
#include <rattle/parser/parser.hpp>
#include <rattle/rattle.hpp>
#include <rattle/tree/nodes.hpp>

namespace rattle::parser::internal {
  namespace expr {
    // How to handle op position
    // * Pass left expr and state which should return binary expression
    typedef Scoped<tree::Expr> (*BinaryParser)(
      Parser &, Scoped<tree::Expr>, Prec) noexcept;
    // * Pass state with the cursor pointing to the op and let it parse its operand
    typedef Scoped<tree::Expr> (*UnaryParser)(Parser &, Prec) noexcept;

    // Holds info on how to react when a token is met, depending
    // on context (where it is and precedence), treat it as a binary op or unary op.
    struct Spec {
      // Pair, parse function and precedence, to parse an expression.
      struct UPair {
        UnaryParser fn;
        Prec prec;
      } unary;
      struct BPair {
        BinaryParser fn;
        Prec prec;
      } binary;
    };

    // Parser functions
    static Scoped<tree::Expr> literal(Parser &, Prec) noexcept;
    static Scoped<tree::Expr> unary_expr(Parser &, Prec) noexcept;
    static Scoped<tree::Expr> isinif_expr(
      Parser &, Scoped<tree::Expr>, Prec) noexcept;
    static Scoped<tree::Expr> call_expr(
      Parser &, Scoped<tree::Expr>, Prec) noexcept;
    static Scoped<tree::Expr> cmp_expr(
      Parser &, Scoped<tree::Expr>, Prec) noexcept;
    static Scoped<tree::Expr> binary_expr(
      Parser &, Scoped<tree::Expr>, Prec) noexcept;

    static Spec get_kind_spec(token::Kind kind) noexcept {
      // clang-format off
      switch(kind) {
        case token::Kind::Plus:          return {{unary_expr, Prec::posify},    {binary_expr, Prec::add}};
        case token::Kind::Minus:         return {{unary_expr, Prec::negate},    {binary_expr, Prec::subtract}};
        case token::Kind::Star:          return {{unary_expr, Prec::spread},    {binary_expr, Prec::multiply}};
        case token::Kind::Slash:         return {{nullptr,    Prec::none},      {binary_expr, Prec::divide}};
        case token::Kind::And:           return {{nullptr,    Prec::none},      {binary_expr, Prec::logic_and}};
        case token::Kind::Or:            return {{nullptr,    Prec::none},      {binary_expr, Prec::logic_or}};
        case token::Kind::Not:           return {{unary_expr, Prec::logic_not}, {isinif_expr, Prec::member_in}};
        case token::Kind::OpenBracket:   return {{unary_expr, Prec::primary},   {call_expr,   Prec::subscript}};
        case token::Kind::OpenParen:     return {{unary_expr, Prec::primary},   {call_expr,   Prec::call}};
        case token::Kind::Dot:           return {{nullptr,    Prec::none},      {call_expr,   Prec::dot}};
        case token::Kind::EqualEqual:    return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare_eq}};
        case token::Kind::NotEqual:      return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare_ne}};
        case token::Kind::LessEqual:     return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case token::Kind::LessThan:      return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case token::Kind::GreaterEqual:  return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case token::Kind::GreaterThan:   return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case token::Kind::In:            return {{nullptr,    Prec::none},      {isinif_expr, Prec::member_in}};
        case token::Kind::Is:            return {{nullptr,    Prec::none},      {isinif_expr, Prec::identity_is}};
        case token::Kind::If:            return {{nullptr,    Prec::none},      {isinif_expr, Prec::if_else}};
        case token::Kind::Comma:         return {{nullptr,    Prec::none},      {binary_expr, Prec::comma}};
#define rattle_undef_token_macro
#define rattle_pp_token_macro(kind, _) \
        case token::Kind::kind:          return {{literal,    Prec::primary},   {nullptr,     Prec::none}};
        rattle_pp_token_macro(Error, _)
#include <rattle/token/require_pp/primitives.h>
#include <rattle/token/require_pp/undefine.h>
        default:                         return {{nullptr,    Prec::none},      {nullptr,     Prec::none}};
      }
      // clang-format on
    }

    static Spec get_kind_spec(Parser &parser) noexcept {
      return get_kind_spec(parser.base.peek().kind);
    }

    // Pratt's expression parsing algorithm
    static Scoped<tree::Expr> pratts_parse_expr(Parser &parser, Prec lowest) {
      auto spec = get_kind_spec(parser);
      Scoped<tree::Expr> left;
      if (spec.unary.prec < lowest) {
        return left;
      }
      if (spec.unary.fn != nullptr) {
        left = spec.unary.fn(parser, spec.unary.prec);
      }
      for (;;) {
        spec = get_kind_spec(parser);
        if (spec.binary.prec < lowest) {
          break;
        }
        if (spec.binary.fn != nullptr) {
          left = spec.binary.fn(parser, std::move(left), spec.binary.prec);
        } else {
          break;
        }
      }
      return left;
    }

    // Parse literals into `Primary` nodes
    static Scoped<tree::Expr> literal(Parser &parser, Prec) noexcept {
      switch (parser.base.iskind()) {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(kind, _) case token::Kind::kind:
        rattle_pp_token_macro(Error, _)
#include <rattle/token/require_pp/primitives.h>
#include <rattle/token/require_pp/undefine.h>
          return parser.make<tree::expr::Primary>(parser.base.eat());
      default:
        unreachable();
      }
    }

    // Parse unary operators
    static Scoped<tree::Expr> unary_expr(Parser &parser, Prec prec) noexcept {
      switch (parser.base.iskind()) {
      case token::Kind::Plus: {
        auto op = parser.base.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::UnaryExpr>(op, std::move(operand));
      }
      case token::Kind::Minus: {
        auto op = parser.base.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::UnaryExpr>(op, std::move(operand));
      }
      case token::Kind::Star: {
        auto op = parser.base.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::UnaryExpr>(op, std::move(operand));
      }
      case token::Kind::Not: {
        auto op = parser.base.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::UnaryExpr>(op, std::move(operand));
      }
      case token::Kind::OpenBracket: {
        auto flags = parser.base.with(State::NEWLINE, State::ALL);
        auto op = parser.base.eat();
        auto operand = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.base.iskind(token::Kind::CloseBracket)) {
          op2 = parser.base.eat();
        } else {
          parser.base.report(error::Kind::unterminated_bracket, op);
        }
        return parser.make<tree::expr::Group>(op, op2, std::move(operand));
      }
      case token::Kind::OpenParen: {
        auto flags = parser.base.with(State::NEWLINE, State::ALL);
        auto op = parser.base.eat();
        auto operand = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.base.iskind(token::Kind::CloseParen)) {
          op2 = parser.base.eat();
        } else {
          parser.base.report(error::Kind::unterminated_paren, op);
        }
        return parser.make<tree::expr::Group>(op, op2, std::move(operand));
      }
      default:
        unreachable();
      }
    }

    // ensure ++prec for left and prec or --prec for right
    template <token::Kind kind, class NodeType>
    static Scoped<tree::Expr> associativity(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) {
      while (parser.base.iskind(kind)) {
        auto op = parser.base.eat();
        auto right = pratts_parse_expr(parser, prec);
        left = parser.make<NodeType>(op, std::move(left), std::move(right));
      }
      return left;
    }

    // Parse binary operators
    static Scoped<tree::Expr> binary_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.base.iskind()) {
      // Left associative
      case token::Kind::Plus:
        return associativity<token::Kind::Plus, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::Minus:
        return associativity<token::Kind::Minus, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::Star:
        return associativity<token::Kind::Star, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::Slash:
        return associativity<token::Kind::Slash, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::Comma:
        return associativity<token::Kind::Comma, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::And:
        return associativity<token::Kind::And, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::Or:
        return associativity<token::Kind::Or, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      default:
        unreachable();
      }
    }

    // Parse compare operators.
    static Scoped<tree::Expr> cmp_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.base.iskind()) {
      case token::Kind::EqualEqual:
        return associativity<token::Kind::EqualEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::NotEqual:
        return associativity<token::Kind::NotEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::LessEqual:
        return associativity<token::Kind::LessEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::LessThan:
        return associativity<token::Kind::LessThan, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::GreaterEqual:
        return associativity<token::Kind::GreaterEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case token::Kind::GreaterThan:
        return associativity<token::Kind::GreaterThan, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      default:
        unreachable();
      }
    }

    // Parse misc operators; like ifelse, is, isnot, in, notin
    static Scoped<tree::Expr> isinif_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.base.iskind()) {
      case token::Kind::Is: {
        auto op = parser.base.eat();
        if (parser.base.iskind(token::Kind::Not)) {
          auto op2 = parser.base.eat();
          auto right = pratts_parse_expr(parser, prec);
          return parser.make<tree::expr::IsNot>(
            op, op2, std::move(left), std::move(right));
        } else {
          auto right = pratts_parse_expr(parser, prec);
          return parser.make<tree::expr::BinaryExpr>(
            op, std::move(left), std::move(right));
        }
      }
      case token::Kind::Not: {
        auto op = parser.base.eat();
        auto op2 = op;
        if (parser.base.iskind(token::Kind::In)) {
          op2 = parser.base.eat();
        } else {
          parser.base.report(error::Kind::patial_notin_operator, op);
        }
        auto right = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::NotIn>(
          op, op2, std::move(left), std::move(right));
      }
      case token::Kind::In: {
        auto op = parser.base.eat();
        auto right = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::In>(
          op, std::move(left), std::move(right));
      }
      case token::Kind::If: {
        auto op = parser.base.eat();
        auto cond = pratts_parse_expr(parser, ++prec);
        if (parser.base.iskind(token::Kind::Else)) {
          auto op2 = parser.base.eat();
          auto right = pratts_parse_expr(parser, prec);
          return parser.make<tree::expr::IfElse>(
            op, op2, std::move(left), std::move(cond), std::move(right));
        } else {
          parser.base.report(error::Kind::patial_ifelse_operator, op);
        }
        auto right = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::IfElse>(
          op, op, std::move(left), std::move(cond), std::move(right));
      }
      default:
        unreachable();
      }
    }

    // Parse call expressions
    static Scoped<tree::Expr> call_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.base.iskind()) {
      case token::Kind::Dot: {
        auto op = parser.base.eat();
        return parser.make<tree::expr::Dot>(op, std::move(left),
          parser.base.iskind(token::Kind::Identifier) ? parser.base.eat() :
                                                        parser.base.peek());
      }
      case token::Kind::OpenBracket: {
        auto flags = parser.base.with(State::NEWLINE, State::ALL);
        auto op = parser.base.eat();
        auto arguments = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.base.iskind(token::Kind::CloseBracket)) {
          op2 = parser.base.eat();
        } else {
          parser.base.report(error::Kind::unterminated_bracket, op);
        }
        return parser.make<tree::expr::Call>(
          op, op2, std::move(left), std::move(arguments));
      }
      case token::Kind::OpenParen: {
        auto flags = parser.base.with(State::NEWLINE, State::ALL);
        auto op = parser.base.eat();
        auto arguments = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.base.iskind(token::Kind::CloseParen)) {
          op2 = parser.base.eat();
        } else {
          parser.base.report(error::Kind::unterminated_paren, op);
        }
        return parser.make<tree::expr::Call>(
          op, op2, std::move(left), std::move(arguments));
      }
      default:
        unreachable();
      }
    }

    static Scoped<tree::Expr> parse_expression(Parser &parser) noexcept {
      // Reset the filter to default setting.
      auto flags = parser.base.with(internal::State::DEFAULT);
      // Parse from lowest precedence.
      return pratts_parse_expr(parser, Prec::lowest);
    }
  } // namespace expr

  // A facade; give the expression parser implementation control and let it do the magic.
  Scoped<tree::Expr> Parser::parse_expression() noexcept {
    return expr::parse_expression(*this);
  }
} // namespace rattle::parser::internal

