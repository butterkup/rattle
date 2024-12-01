#include "parser.hpp"
#include "rattle/parser_nodes.hpp"
#include <memory>
#include <rattle/parser.hpp>

namespace rattle::parser {
  std::unique_ptr<nodes::Statement> parse_statement(State &state) {}

  std::unique_ptr<nodes::Block> block_stmt(State &state,
                                           lexer::Token const &brace) {
    std::vector<std::unique_ptr<nodes::Statement>> stmts;
    while (true) {
      if (state.empty()) {
        state.report(error_t::unterminated_brace, brace);
      }
      auto token = get(state);
      if (token.kind == lexer::Token::Kind::CloseBrace) {
        break;
      }
      state.unget(token);
      stmts.emplace_back(parse_statement(state));
    }
    stmts.shrink_to_fit();
    return std::make_unique<nodes::Block>(std::move(stmts));
  }

  std::unique_ptr<nodes::Class> class_stmt(State &state,
                                           lexer::Token const &klass) {
    auto name = get(state);
    switch (name.kind) {
    case lexer::Token::Kind::Identifier:
      break;
    default:
      state.report(error_t::expected_identifier, name);
      state.unget(name);
    }
    auto paren = get(state);
    std::unique_ptr<nodes::Expression> bases;
    if (paren.kind == lexer::Token::Kind::OpenParen) {
      bases = (state.unget(paren), parse_expression(state));
    }
    auto brace = get(state);
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::Class>(name, std::move(bases),
                                          std::move(body));
  }

  std::unique_ptr<nodes::Statement> fn_stmt(State &state,
                                            lexer::Token const &fn) {
    auto name = get(state);
    switch (name.kind) {
    case lexer::Token::Kind::OpenParen:
      return (state.unget(name), state.unget(fn), parse_expression(state));
    case lexer::Token::Kind::Identifier:
      break;
    default:
      state.report(error_t::expected_identifier, name);
      state.unget(name);
    }
    auto paren = get(state);
    std::unique_ptr<nodes::Expression> params;
    switch (paren.kind) {
    case lexer::Token::Kind::OpenParen:
      params = (state.unget(paren), parse_expression(state));
    default:
      state.report(error_t::expected_params, paren);
      state.unget(paren);
    }
    auto brace = get(state);
    std::unique_ptr<nodes::Block> body;
    switch (brace.kind) {
    case lexer::Token::Kind::OpenBrace:
      body = block_stmt(state, brace);
    default:
      state.report(error_t::expected_block, brace);
      state.unget(brace);
    }
    return std::make_unique<nodes::Fn>(name, std::move(params),
                                       std::move(body));
  }
} // namespace rattle::parser
