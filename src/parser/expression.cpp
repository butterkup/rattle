#include "precedence.hpp"
#include "rattle/token/token.hpp"
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

    static Spec get_kind_spec(
      token::kinds::Token kind, int flags = 0) noexcept {
      // clang-format off
      switch(rattle_merge_kind(kind, flags)) {
        case rattle_merge_kind2(Operator, Plus):          return {{unary_expr, Prec::posify},    {binary_expr, Prec::add}};
        case rattle_merge_kind2(Operator, Minus):         return {{unary_expr, Prec::negate},    {binary_expr, Prec::subtract}};
        case rattle_merge_kind2(Operator, Star):          return {{unary_expr, Prec::spread},    {binary_expr, Prec::multiply}};
        case rattle_merge_kind2(Operator, Slash):         return {{nullptr,    Prec::none},      {binary_expr, Prec::divide}};
        case rattle_merge_kind2(Operator, Dot):           return {{nullptr,    Prec::none},      {call_expr,   Prec::dot}};
        case rattle_merge_kind2(Operator, EqualEqual):    return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare_eq}};
        case rattle_merge_kind2(Operator, NotEqual):      return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare_ne}};
        case rattle_merge_kind2(Operator, LessEqual):     return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case rattle_merge_kind2(Operator, LessThan):      return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case rattle_merge_kind2(Operator, GreaterEqual):  return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case rattle_merge_kind2(Operator, GreaterThan):   return {{nullptr,    Prec::none},      {cmp_expr,    Prec::compare}};
        case rattle_merge_kind2(Operator, Comma):         return {{nullptr,    Prec::none},      {binary_expr, Prec::comma}};
        case rattle_merge_kind2(Identifier, In):          return {{nullptr,    Prec::none},      {isinif_expr, Prec::member_in}};
        case rattle_merge_kind2(Identifier, Is):          return {{nullptr,    Prec::none},      {isinif_expr, Prec::identity_is}};
        case rattle_merge_kind2(Identifier, If):          return {{nullptr,    Prec::none},      {isinif_expr, Prec::if_else}};
        case rattle_merge_kind2(Identifier, And):         return {{nullptr,    Prec::none},      {binary_expr, Prec::logic_and}};
        case rattle_merge_kind2(Identifier, Or):          return {{nullptr,    Prec::none},      {binary_expr, Prec::logic_or}};
        case rattle_merge_kind2(Identifier, Not):         return {{unary_expr, Prec::logic_not}, {isinif_expr, Prec::member_in}};
        case rattle_merge_kind2(Marker, OpenBracket):     return {{unary_expr, Prec::primary},   {call_expr,   Prec::subscript}};
        case rattle_merge_kind2(Marker, OpenParen):       return {{unary_expr, Prec::primary},   {call_expr,   Prec::call}};
        default:
          switch (kind) {
            case token::kinds::Token::Number:
            case token::kinds::Token::String:
            case token::kinds::Token::Identifier:         return {{literal,    Prec::primary},   {nullptr,     Prec::none}};
            default:                                      return {{nullptr,    Prec::none},      {nullptr,     Prec::none}};
          }
      }
      // clang-format on
    }

    static Spec get_kind_spec(Parser &parser, int flags = 0) noexcept {
      return get_kind_spec(parser.cursor.peek().kind, flags);
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
      if (parser.cursor.iskind(token::kinds::Token::Number) or
          parser.cursor.iskind(token::kinds::Token::String)) {
        return parser.make<tree::expr::Literal>(parser.cursor.eat());
      }
      switch (parser.cursor.merge()) {
#define rattle_pp_token_macro(Keyword, _)                                      \
  case rattle_merge_kind2(Identifier, Keyword):
#include <rattle/token/require_pp/keywords/literal.h>
        rattle_pp_token_macro(Variable, _)
#undef rattle_pp_token_macro
          return parser.make<tree::expr::Literal>(parser.cursor.eat());
      default:
        unreachable();
      }
    }

    // Parse unary operators
    static Scoped<tree::Expr> unary_expr(Parser &parser, Prec prec) noexcept {
      switch (parser.cursor.merge()) {
      case rattle_merge_kind2(Operator, Plus): {
        auto op = parser.cursor.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::BinaryExpr>(
          op, nullptr, std::move(operand));
      }
      case rattle_merge_kind2(Operator, Minus): {
        auto op = parser.cursor.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::BinaryExpr>(
          op, nullptr, std::move(operand));
      }
      case rattle_merge_kind2(Operator, Star): {
        auto op = parser.cursor.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::BinaryExpr>(
          op, nullptr, std::move(operand));
      }
      case rattle_merge_kind2(Identifier, Not): {
        auto op = parser.cursor.eat();
        auto operand = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::BinaryExpr>(
          op, nullptr, std::move(operand));
      }
      case rattle_merge_kind2(Marker, OpenBracket): {
        auto flags = parser.cursor.with(State::NEWLINE, State::ALL);
        auto op = parser.cursor.eat();
        auto operand = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.cursor.iskind(token::kinds::Marker::CloseBracket)) {
          op2 = parser.cursor.eat();
        } else {
          parser.cursor.report(error::Kind::unterminated_bracket, op);
        }
        return parser.make<tree::expr::BiExprBiTk>(
          op, op2, nullptr, std::move(operand));
      }
      case rattle_merge_kind2(Marker, OpenParen): {
        auto flags = parser.cursor.with(State::NEWLINE, State::ALL);
        auto op = parser.cursor.eat();
        auto operand = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.cursor.iskind(token::kinds::Marker::CloseParen)) {
          op2 = parser.cursor.eat();
        } else {
          parser.cursor.report(error::Kind::unterminated_paren, op);
        }
        return parser.make<tree::expr::BiExprBiTk>(
          op, op2, nullptr, std::move(operand));
      }
      default:
        unreachable();
      }
    }

    // ensure ++prec for left and prec or --prec for right
    template <token::kinds::Token kind, int flags, class NodeType>
    static Scoped<tree::Expr> associativity(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) {
      while (parser.cursor.iskind(kind, flags)) {
        auto op = parser.cursor.eat();
        auto right = pratts_parse_expr(parser, prec);
        left = parser.make<NodeType>(op, std::move(left), std::move(right));
      }
      return left;
    }

    // Parse binary operators
    static Scoped<tree::Expr> binary_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.cursor.merge()) {
      // Left associative
      case rattle_merge_kind2(Operator, Plus):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::Plus, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, Minus):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::Minus, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, Star):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::Star, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, Slash):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::Slash, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, Comma):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::Comma, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Identifier, And):
        return associativity<token::kinds::Token::Identifier,
          token::kinds::Identifier::And, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Identifier, Or):
        return associativity<token::kinds::Token::Identifier,
          token::kinds::Identifier::Or, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      default:
        unreachable();
      }
    }

    // Parse compare operators.
    static Scoped<tree::Expr> cmp_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.cursor.merge()) {
      case rattle_merge_kind2(Operator, EqualEqual):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::EqualEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, NotEqual):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::NotEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, LessEqual):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::LessEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, LessThan):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::LessThan, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, GreaterEqual):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::GreaterEqual, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      case rattle_merge_kind2(Operator, GreaterThan):
        return associativity<token::kinds::Token::Operator,
          token::kinds::Operator::GreaterThan, tree::expr::BinaryExpr>(
          parser, std::move(left), ++prec);
      default:
        unreachable();
      }
    }

    // Parse misc operators; like ifelse, is, isnot, in, notin
    static Scoped<tree::Expr> isinif_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.cursor.merge()) {
      case rattle_merge_kind2(Identifier, Is): {
        auto op = parser.cursor.eat();
        if (parser.cursor.iskind(token::kinds::Identifier::Not)) {
          auto op2 = parser.cursor.eat();
          auto right = pratts_parse_expr(parser, prec);
          return parser.make<tree::expr::BiExprBiTk>(
            op, op2, std::move(left), std::move(right));
        } else {
          auto right = pratts_parse_expr(parser, prec);
          return parser.make<tree::expr::BinaryExpr>(
            op, std::move(left), std::move(right));
        }
      }
      case rattle_merge_kind2(Identifier, Not): {
        auto op = parser.cursor.eat();
        auto op2 = op;
        if (parser.cursor.iskind(token::kinds::Identifier::In)) {
          op2 = parser.cursor.eat();
        } else {
          parser.cursor.report(error::Kind::patial_notin_operator, op);
        }
        auto right = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::BiExprBiTk>(
          op, op2, std::move(left), std::move(right));
      }
      case rattle_merge_kind2(Identifier, In): {
        auto op = parser.cursor.eat();
        auto right = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::BinaryExpr>(
          op, std::move(left), std::move(right));
      }
      case rattle_merge_kind2(Identifier, If): {
        auto op = parser.cursor.eat();
        auto cond = pratts_parse_expr(parser, ++prec);
        if (parser.cursor.iskind(token::kinds::Identifier::Else)) {
          auto op2 = parser.cursor.eat();
          auto right = pratts_parse_expr(parser, prec);
          return parser.make<tree::expr::BiExprBiTk>(op, op2,
            parser.make<tree::expr::BinaryExpr>(
              op, std::move(left), std::move(cond)),
            std::move(right));
        } else {
          parser.cursor.report(error::Kind::patial_ifelse_operator, op);
        }
        auto right = pratts_parse_expr(parser, prec);
        return parser.make<tree::expr::BiExprBiTk>(op, op,
          parser.make<tree::expr::BinaryExpr>(
            op, std::move(left), std::move(cond)),
          std::move(right));
      }
      default:
        unreachable();
      }
    }

    // Parse call expressions
    static Scoped<tree::Expr> call_expr(
      Parser &parser, Scoped<tree::Expr> left, Prec prec) noexcept {
      switch (parser.cursor.merge()) {
      case rattle_merge_kind2(Operator, Dot): {
        auto op = parser.cursor.eat();
        return parser.make<tree::expr::BinaryExpr>(
          op, std::move(left), pratts_parse_expr(parser, Prec::primary));
      }
      case rattle_merge_kind2(Marker, OpenBracket): {
        auto flags = parser.cursor.with(State::NEWLINE, State::ALL);
        auto op = parser.cursor.eat();
        auto arguments = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.cursor.iskind(token::kinds::Marker::CloseBracket)) {
          op2 = parser.cursor.eat();
        } else {
          parser.cursor.report(error::Kind::unterminated_bracket, op);
        }
        return parser.make<tree::expr::BiExprBiTk>(
          op, op2, std::move(left), std::move(arguments));
      }
      case rattle_merge_kind2(Marker, OpenParen): {
        auto flags = parser.cursor.with(State::NEWLINE, State::ALL);
        auto op = parser.cursor.eat();
        auto arguments = pratts_parse_expr(parser, Prec::lowest);
        auto op2 = op;
        if (parser.cursor.iskind(token::kinds::Marker::CloseParen)) {
          op2 = parser.cursor.eat();
        } else {
          parser.cursor.report(error::Kind::unterminated_paren, op);
        }
        return parser.make<tree::expr::BiExprBiTk>(
          op, op2, std::move(left), std::move(arguments));
      }
      default:
        unreachable();
      }
    }

    static Scoped<tree::Expr> parse_expression(Parser &parser) noexcept {
      // Reset the filter to default setting.
      auto flags = parser.cursor.with(internal::State::DEFAULT);
      // Parse from lowest precedence.
      return pratts_parse_expr(parser, Prec::lowest);
    }
  } // namespace expr

  // A facade; give the expression parser implementation control and let it do the magic.
  Scoped<tree::Expr> Parser::parse_expression() noexcept {
    return expr::parse_expression(*this);
  }
} // namespace rattle::parser::internal

