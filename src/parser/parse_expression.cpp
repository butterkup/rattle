#include "precedence.hpp"
#include "rattle/lexer.hpp"
#include "rattle/node.hpp"
#include "rattle/parser.hpp"
#include <memory>
#include <utility>

namespace rattle::parser {
  using UniqueExpr = std::unique_ptr<nodes::Expression>;
  typedef UniqueExpr (*ParseUnary)(State &, prec, lexer::Token &);
  typedef UniqueExpr (*ParseBinary)(State &, prec, lexer::Token &, UniqueExpr);

#define _decl_unary(_name)                                                     \
  static UniqueExpr _name(State &state, prec prec, lexer::Token &token)
#define _decl_binary(_name)                                                    \
  static UniqueExpr _name(State &state, prec prec, lexer::Token &)

  _decl_unary(parse_primary);
  _decl_unary(parse_group);

  struct PrecParser {
    prec binary_prec, unary_prec;
    ParseUnary unary_parser;
    ParseBinary binary_parser;
  };

  // clang-format off
  static PrecParser get_token_prec(lexer::Token::Kind kind) {
    switch(kind) {
      case lexer::Token::Kind::Plus:          return {prec::plus,        prec::uplus,    nullptr,         nullptr};
      case lexer::Token::Kind::Minus:         return {prec::minus,       prec::uminus,   nullptr,         nullptr};
      case lexer::Token::Kind::Star:          return {prec::multiply,    prec::none,     nullptr,         nullptr};
      case lexer::Token::Kind::Slash:         return {prec::divide,      prec::none,     nullptr,         nullptr};
      case lexer::Token::Kind::Dot:           return {prec::dot,         prec::none,     nullptr,         nullptr};
      case lexer::Token::Kind::OpenParen:     return {prec::call,        prec::group,    parse_group,     nullptr};
      case lexer::Token::Kind::OpenBracket:   return {prec::subscript,   prec::none,     nullptr,         nullptr};
      case lexer::Token::Kind::Floating:      return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::Binary:        return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::Decimal:       return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::Octal:         return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::Hexadecimal:   return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::String:        return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::True:          return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::None:          return {prec::none,        prec::primary,  parse_primary,   nullptr};
      case lexer::Token::Kind::False:         return {prec::none,        prec::primary,  parse_primary,   nullptr};
      default:                                return {prec::none,        prec::none,     nullptr,         nullptr};
    }
  }
  // clang-format on

  static UniqueExpr _parse_expression(State &state, prec threshold) {
    UniqueExpr expression;
    if (state.empty()) {
      return expression;
    }
    auto token = state.get();
    auto parser = get_token_prec(token.kind);
    if (parser.unary_prec > threshold) {
      if (parser.unary_parser) {
        expression = parser.unary_parser(state, parser.unary_prec, token);
      } else {
        state.report(error_t::expected_an_expression, token);
      }
    } else {
      state.unget(token);
      return expression;
    }
    while (true) {
      token = state.get();
      parser = get_token_prec(token.kind);
      if (parser.binary_prec > threshold && parser.binary_parser) {
        expression = parser.binary_parser(state, parser.binary_prec, token,
                                          std::move(expression));
      } else {
        break;
      }
    }
    return expression;
  }

  UniqueExpr parse_expression(State &state) {
    return _parse_expression(state, prec::_lowest);
  }

  _decl_unary(parse_primary) {
    using K = lexer::Token::Kind;
    namespace N = nodes;
    auto b = std::make_unique<N::LiteralExpr>(token);
    switch (token.kind) {
    case K::Binary:
    case K::Floating:
    case K::Hexadecimal:
    case K::Octal:
    case K::Decimal:
      return std::make_unique<N::Number>(token);
    case K::String:
      return std::make_unique<N::String>(token);
    case K::True:
      return std::make_unique<N::True>(token);
    case K::False:
      return std::make_unique<N::False>(token);
    case K::None:
      return std::make_unique<N::None>(token);
    default:
      return {};
    }
  }

  _decl_unary(parse_group) {
    using K = lexer::Token::Kind;
    auto operand = parse_expression(state);
    if (operand.get() == nullptr) {
      state.report(error_t::expected_an_expression, token);
    }
    auto rparen = state.get();
    if (rparen.kind != K::CloseParen) {
      state.report(error_t::unterminated_paren, token);
      state.unget(rparen);
    }
    return std::make_unique<nodes::Group>(token, std::move(operand));
  }
} // namespace rattle::parser

