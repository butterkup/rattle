#ifndef RATTLE_SOURCE_ONLY
#include "parser.hpp"
#include "precedence.hpp"
#include <cassert>
#include <memory>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>
#include <rattle/parser_nodes.hpp>
#include <utility>
#endif

namespace rattle::parser {
#define UniqueExpr std::unique_ptr<nodes::Expression>
  typedef UniqueExpr (*ParseUnary)(State &, prec, lexer::Token &);
  typedef UniqueExpr (*ParseBinary)(State &, prec, lexer::Token &, UniqueExpr);

#define _decl_unary(_name)                                                     \
  static UniqueExpr _name(State &state, prec prec, lexer::Token &token)
#define _decl_binary(_name)                                                    \
  static UniqueExpr _name(State &state, prec prec, lexer::Token &token,        \
                          UniqueExpr left)

  _decl_unary(parse_primary);
  _decl_unary(parse_paren);
  _decl_unary(parse_brace);
  _decl_unary(parse_bracket);
  _decl_unary(parse_unary);
  _decl_binary(parse_binary);
  _decl_binary(parse_multik);

  struct PrecParser {
    prec unary_prec;
    ParseUnary unary_parser;
    prec binary_prec;
    ParseBinary binary_parser;
  };

  // clang-format off
  static PrecParser get_token_prec(lexer::Token::Kind kind) {
    using Kind = lexer::Token::Kind;
    switch(kind) {
      case Kind::Lshift:       return {prec::none,       nullptr,        prec::shift,      parse_binary};
      case Kind::Rshift:       return {prec::none,       nullptr,        prec::shift,      parse_binary};
      case Kind::NotEqual:     return {prec::none,       nullptr,        prec::not_equal,  parse_binary};
      case Kind::EqualEqual:   return {prec::none,       nullptr,        prec::equal,      parse_binary};
      case Kind::LessEqual:    return {prec::none,       nullptr,        prec::compare,    parse_binary};
      case Kind::GreaterEqual: return {prec::none,       nullptr,        prec::compare,    parse_binary};
      case Kind::Less:         return {prec::none,       nullptr,        prec::compare,    parse_binary};
      case Kind::Greater:      return {prec::none,       nullptr,        prec::compare,    parse_binary};
      case Kind::Dot:          return {prec::none,       nullptr,        prec::dot,        parse_binary};
      case Kind::Xor:          return {prec::none,       nullptr,        prec::xor_,       parse_binary};
      case Kind::Plus:         return {prec::uplus,      parse_unary,    prec::plus,       parse_binary};
      case Kind::Minus:        return {prec::uminus,     parse_unary,    prec::minus,      parse_binary};
      case Kind::Star:         return {prec::none,       nullptr,        prec::multiply,   parse_binary};
      case Kind::Slash:        return {prec::none,       nullptr,        prec::divide,     parse_binary};
      case Kind::Percent:      return {prec::none,       nullptr,        prec::modulus,    parse_binary};
      case Kind::BitAnd:       return {prec::none,       nullptr,        prec::bit_and,    parse_binary};
      case Kind::And:          return {prec::none,       nullptr,        prec::logic_or,   parse_binary};
      case Kind::Or:           return {prec::none,       nullptr,        prec::logic_and,  parse_binary};
      case Kind::BitOr:        return {prec::none,       nullptr,        prec::bit_or,     parse_binary};
      case Kind::At:           return {prec::none,       nullptr,        prec::matmul,     parse_binary};
      case Kind::Invert:       return {prec::invert,     parse_unary,    prec::none,       nullptr     };

      case Kind::Is:           return {prec::none,       nullptr,        prec::is,         parse_multik};
      case Kind::If:           return {prec::none,       nullptr,        prec::if_else,    parse_binary};
      case Kind::In:           return {prec::none,       nullptr,        prec::in,         parse_binary};
      case Kind::Not:          return {prec::logic_not,  parse_unary,    prec::in,         parse_multik};

      case Kind::Fn:           return {prec::primary,    parse_unary,    prec::none,       nullptr     };
      case Kind::Yield:        return {prec::yield,      parse_unary,    prec::none,       nullptr     };
      case Kind::As:           return {prec::none,       nullptr,        prec::as,         parse_binary};
      case Kind::Colon:        return {prec::colon,      parse_unary,    prec::colon,      parse_binary};
      case Kind::Comma:        return {prec::comma,      parse_unary,    prec::comma,      parse_binary};
      case Kind::OpenParen:    return {prec::primary,    parse_paren,    prec::call,       parse_binary};
      case Kind::OpenBracket:  return {prec::primary,    parse_bracket,  prec::subscript,  parse_binary};
      case Kind::OpenBrace:    return {prec::primary,    parse_brace,    prec::none,       nullptr     };

      case Kind::Floating:     return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::Binary:       return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::Decimal:      return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::Octal:        return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::Hexadecimal:  return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::String:       return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::Identifier:   return {prec::primary,    parse_primary,  prec::none,       nullptr     };

      case Kind::True:         return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::None:         return {prec::primary,    parse_primary,  prec::none,       nullptr     };
      case Kind::False:        return {prec::primary,    parse_primary,  prec::none,       nullptr     };

      case Kind::Error:        return {prec::_lowest,    parse_unary,    prec::_lowest,    parse_binary};
      default:                 return {prec::none,       nullptr,        prec::none,       nullptr     };
    }
  }
  // clang-format on

  static UniqueExpr _parse_expression(State &state, prec threshold) {
    UniqueExpr expression;
    auto token = state.get();
    auto parser = get_token_prec(token.kind);
    if (parser.unary_prec >= threshold) {
      assert(parser.unary_parser);
      expression = parser.unary_parser(state, parser.unary_prec, token);
    } else {
      state.unget(token);
      return expression;
    }
    while (true) {
      token = state.get();
      parser = get_token_prec(token.kind);
      if (parser.binary_prec >= threshold) {
        assert(parser.binary_parser);
        expression = parser.binary_parser(state, parser.binary_prec, token,
                                          std::move(expression));
      } else {
        state.unget(token);
        break;
      }
    }
    return expression;
  }

  UniqueExpr parse_expression(State &state) {
    auto ctx = state.with();
    return _parse_expression(state, prec::_lowest);
  }

  _decl_unary(parse_primary) {
    switch (token.kind) {
    case lexer::Token::Kind::Binary:
    case lexer::Token::Kind::Floating:
    case lexer::Token::Kind::Hexadecimal:
    case lexer::Token::Kind::Octal:
    case lexer::Token::Kind::Decimal:
      return std::make_unique<nodes::Number>(token);
    case lexer::Token::Kind::Identifier:
      return std::make_unique<nodes::Identifier>(token);
    case lexer::Token::Kind::String:
      return std::make_unique<nodes::String>(token);
    case lexer::Token::Kind::True:
      return std::make_unique<nodes::True>(token);
    case lexer::Token::Kind::False:
      return std::make_unique<nodes::False>(token);
    case lexer::Token::Kind::None:
      return std::make_unique<nodes::None>(token);
    default:
      assert(false);
    }
  }

#define parse_right _parse_expression(state, prec)

  _decl_unary(parse_unary) {
    switch (token.kind) {
    case lexer::Token::Kind::Colon:
    case lexer::Token::Kind::Comma:
      return std::make_unique<nodes::Separator>(token, nullptr, parse_right);
    case lexer::Token::Kind::Minus:
      return std::make_unique<nodes::Minus>(token, nullptr, parse_right);
    case lexer::Token::Kind::Plus:
      return std::make_unique<nodes::Plus>(token, nullptr, parse_right);
    case lexer::Token::Kind::Error:
      return std::make_unique<nodes::UnaryExpr>(token, parse_right);
    case lexer::Token::Kind::Yield:
      return std::make_unique<nodes::Yield>(token, parse_right);
    case lexer::Token::Kind::Not:
      return std::make_unique<nodes::Not>(token, parse_right);
    case lexer::Token::Kind::Fn:
      return fn_expression(state, token);
    default:
      assert(false);
    }
  }

  _decl_binary(parse_binary) {
    switch (token.kind) {
#define TK_MACRO(_Name, _)                                                     \
  case lexer::Token::Kind::_Name:                                              \
    return std::make_unique<nodes::_Name>(token, std::move(left), parse_right);
#define TK_INCLUDE (TK_OPALL | TK_KEYBINARY)
#include <rattle/token_macro.hpp>
    case lexer::Token::Kind::OpenBracket: {
      auto scope = state.enter_bracket();
      auto ctx = state.with(State::IGNORE_NLCOM);
      auto arguments = _parse_expression(state, prec::_lowest);
      auto rbracket = state.get();
      if (rbracket.kind != lexer::Token::Kind::CloseBracket) {
        state.report(error_t::unterminated_bracket, token);
        state.unget(rbracket);
      }
      return std::make_unique<nodes::Subscript>(token, std::move(left),
                                                std::move(arguments));
    }
    case lexer::Token::Kind::OpenParen: {
      auto scope = state.enter_paren();
      auto ctx = state.with(State::IGNORE_NLCOM);
      auto arguments = _parse_expression(state, prec::_lowest);
      auto rparen = state.get();
      if (rparen.kind != lexer::Token::Kind::CloseParen) {
        state.report(error_t::unterminated_paren, token);
        state.unget(rparen);
      }
      return std::make_unique<nodes::Call>(token, std::move(left),
                                           std::move(arguments));
    }
    case lexer::Token::Kind::If: {
      auto condition = parse_right;
      auto else_tk = state.get();
      UniqueExpr right;
      if (else_tk.kind == lexer::Token::Kind::Else) {
        right = parse_right;
      } else {
        state.report(error_t::unterminated_if_else_expr, else_tk);
        state.unget(else_tk);
      }
      return std::make_unique<nodes::IfElse>(token, std::move(condition),
                                             std::move(left), std::move(right));
    }
    case lexer::Token::Kind::Colon:
    case lexer::Token::Kind::Comma:
      return std::make_unique<nodes::Separator>(token, std::move(left),
                                                parse_right);
    case lexer::Token::Kind::Error:
      return std::make_unique<nodes::BinaryExpr>(token, std::move(left),
                                                 parse_right);
    default:
      assert(false);
    }
  }

  _decl_binary(parse_multik) {
    switch (token.kind) {
    case lexer::Token::Kind::Not: {
      auto token_ = state.get();
      if (token_.kind != lexer::Token::Kind::In) {
        state.report(error_t::incomplete_operator_notin, token);
        state.unget(token_);
        return std::make_unique<nodes::BinaryExpr>(token, std::move(left),
                                                   parse_right);
      }
      token.end = token_.end;
      token.proc.end = token_.proc.end;
      return std::make_unique<nodes::NotIn>(token, std::move(left),
                                            parse_right);
    }
    case lexer::Token::Kind::Is: {
      auto token_ = state.get();
      if (token_.kind == lexer::Token::Kind::Not) {
        token.end = token_.end;
        token.proc.end = token_.proc.end;
        return std::make_unique<nodes::IsNot>(token, std::move(left),
                                              parse_right);
      }
      state.unget(token_);
      return std::make_unique<nodes::Is>(token, std::move(left), parse_right);
    }
    default:
      assert(false);
    }
  }

  template <lexer::Token::Kind close, error_t unclosed>
  _decl_unary(parse_container) {
    auto ctx = state.with(State::IGNORE_NLCOM);
    auto operand = parse_right;
    auto token_ = state.get();
    if (token_.kind != close) {
      state.report(unclosed, token);
      state.unget(token_);
    }
    return std::make_unique<nodes::Container>(
      token, nodes::Container::Type::None, std::move(operand));
  }

#undef parse_right

  _decl_unary(parse_paren) {
    auto scope = state.enter_paren();
    return parse_container<lexer::Token::Kind::CloseParen,
                           error_t::unterminated_paren>(state, prec::_lowest,
                                                        token);
  }

  _decl_unary(parse_bracket) {
    auto scope = state.enter_bracket();
    return parse_container<lexer::Token::Kind::CloseBracket,
                           error_t::unterminated_bracket>(state, prec::_lowest,
                                                          token);
  }

  _decl_unary(parse_brace) {
    auto scope = state.enter_brace();
    return parse_container<lexer::Token::Kind::CloseBrace,
                           error_t::unterminated_brace>(state, prec::_lowest,
                                                        token);
  }
#undef _decl_unary
#undef _decl_binary
} // namespace rattle::parser

