#include "parser.hpp"
#include <memory>
#include <rattle/category.hpp>
#include <rattle/parser.hpp>
#include <rattle/parser_nodes.hpp>

namespace rattle::parser {
  static void expect_eos(State &state) {
    auto ctx = state.with(State::IGNORE_COMMENTS);
    auto token = state.get();
    switch (token.kind) {
    case lexer::Token::Kind::CloseBrace:
      state.unget(token);
    case lexer::Token::Kind::Eos:
    case lexer::Token::Kind::Eot:
      return;
    default:
      state.report(error_t::expected_eos, token);
      state.unget(token);
    }
  }

  static auto expr_stmt(State &state) {
    auto expr = parse_expression(state);
    expect_eos(state);
    return expr;
  }

  static auto continue_stmt(State &state, lexer::Token const &kwd) {
    expect_eos(state);
    return std::make_unique<nodes::Continue>(kwd);
  }

  static auto break_stmt(State &state, lexer::Token const &kwd) {
    expect_eos(state);
    return std::make_unique<nodes::Break>(kwd);
  }

  static auto return_stmt(State &state, lexer::Token const &kwd) {
    return std::make_unique<nodes::Return>(kwd, expr_stmt(state));
  }

  static auto nonlocal_stmt(State &state, lexer::Token const &kwd) {
    return std::make_unique<nodes::NonLocal>(kwd, expr_stmt(state));
  }

  static auto del_stmt(State &state, lexer::Token const &kwd) {
    return std::make_unique<nodes::Del>(kwd, expr_stmt(state));
  }

  static auto global_stmt(State &state, lexer::Token const &kwd) {
    return std::make_unique<nodes::Global>(kwd, expr_stmt(state));
  }

  static auto assert_stmt(State &state, lexer::Token const &kwd) {
    return std::make_unique<nodes::Assert>(kwd, expr_stmt(state));
  }

  static auto raise_stmt(State &state, lexer::Token const &kwd) {
    return std::make_unique<nodes::Raise>(kwd, expr_stmt(state));
  }

  static auto import_stmt(State &state, lexer::Token const &kwd) {
    return std::make_unique<nodes::Import>(kwd, expr_stmt(state));
  }

  static auto block_stmt(State &state, lexer::Token const &brace) {
    std::vector<std::unique_ptr<nodes::Statement>> stmts;
    auto scope = state.enter_brace();
    while (true) {
      if (state.empty()) {
        state.report(error_t::unterminated_brace, brace);
        break;
      }
      {
        auto ctx = state.with(State::IGNORE_EOSCOM);
        auto token = state.get();
        if (token.kind == lexer::Token::Kind::CloseBrace) {
          break;
        }
        state.unget(token);
      }
      stmts.emplace_back(parse_statement(state));
    }
    stmts.shrink_to_fit();
    return std::make_unique<nodes::Block>(brace, std::move(stmts));
  }

  static auto from_stmt(State &state, lexer::Token const &kwd) {
    auto package = parse_expression(state);
    auto token = state.get();
    std::unique_ptr<nodes::Import> module;
    if (token.kind == lexer::Token::Kind::Import) {
      module = import_stmt(state, token);
    } else {
      state.unget(token);
    }
    return std::make_unique<nodes::From>(kwd, std::move(package),
                                         std::move(module));
  }

  static auto except_stmt(State &state, lexer::Token const &kwd) {
    std::unique_ptr<nodes::Expression> captured;
    std::unique_ptr<nodes::Block> handler;
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto token = state.get();
    if (token.kind == lexer::Token::Kind::OpenBrace) {
      handler = block_stmt(state, token);
    } else {
      state.unget(token);
      captured = parse_expression(state);
    }
    token = state.get();
    if (token.kind == lexer::Token::Kind::OpenBrace) {
      handler = block_stmt(state, token);
    } else {
      state.unget(token);
    }
    return std::make_unique<nodes::Except>(kwd, std::move(captured),
                                           std::move(handler));
  }

  static auto lastly_stmt(State &state, lexer::Token const &kwd) {
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto brace = state.get();
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::Lastly>(kwd, std::move(body));
  }

  static auto else_stmt(State &state, lexer::Token const &kwd) {
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto brace = state.get();
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::Else>(kwd, std::move(body));
  }

  static auto try_stmt(State &state, lexer::Token const &kwd) {
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto token = state.get();
    std::unique_ptr<nodes::Block> try_block;
    if (token.kind == lexer::Token::Kind::OpenBrace) {
      try_block = block_stmt(state, token);
    } else {
      state.unget(token);
    }
    std::vector<std::unique_ptr<nodes::Except>> handlers;
    while (true) {
      token = state.get();
      if (token.kind == lexer::Token::Kind::Except) {
        handlers.emplace_back(except_stmt(state, token));
      } else {
        state.unget(token);
        break;
      }
    }
    token = state.get();
    std::unique_ptr<nodes::Else> else_block;
    if (token.kind == lexer::Token::Kind::Else) {
      else_block = else_stmt(state, token);
    } else {
      state.unget(token);
    }
    token = state.get();
    std::unique_ptr<nodes::Lastly> lastly_block;
    if (token.kind == lexer::Token::Kind::Else) {
      lastly_block = lastly_stmt(state, token);
    } else {
      state.unget(token);
    }
    return std::make_unique<nodes::Try>(
      kwd, std::move(handlers), std::move(else_block), std::move(lastly_block));
  }

  static auto with_stmt(State &state, lexer::Token const &kwd) {
    auto contexts = parse_expression(state);
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto brace = state.get();
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::With>(kwd, std::move(contexts),
                                         std::move(body));
  }

  static auto for_stmt(State &state, lexer::Token const &kwd) {
    auto bindings = parse_expression(state);
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto brace = state.get();
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::For>(kwd, std::move(bindings),
                                        std::move(body));
  }

  static auto while_stmt(State &state, lexer::Token const &kwd) {
    auto condition = parse_expression(state);
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto brace = state.get();
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::While>(kwd, std::move(condition),
                                          std::move(body));
  }

  static auto if_stmt(State &state, lexer::Token const &kwd) {
    auto condition = parse_expression(state);
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto token = state.get();
    std::unique_ptr<nodes::Block> body;
    if (token.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, token);
    } else {
      state.unget(token);
    }
    token = state.get();
    std::unique_ptr<nodes::Else> else_block;
    if (token.kind == lexer::Token::Kind::Else) {
      else_block = else_stmt(state, token);
    } else {
      state.unget(token);
    }
    return std::make_unique<nodes::If>(kwd, std::move(condition),
                                       std::move(body), std::move(else_block));
  }

  static auto class_stmt(State &state, lexer::Token const &klass) {
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto name = state.get();
    if (name.kind != lexer::Token::Kind::Identifier) {
      state.unget(name);
      name = klass;
    }
    auto paren = state.get();
    std::unique_ptr<nodes::Expression> bases;
    if (paren.kind == lexer::Token::Kind::OpenParen) {
      bases = (state.unget(paren), parse_expression(state));
    } else {
      state.unget(paren);
    }
    auto brace = state.get();
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::Class>(klass, name, std::move(bases),
                                          std::move(body));
  }

  static std::unique_ptr<nodes::Statement> fn_stmt(State &state,
                                                   lexer::Token const &fn) {
    auto ctx = state.with(State::IGNORE_EOSCOM);
    auto name = state.get();
    switch (name.kind) {
    case lexer::Token::Kind::OpenParen: {
      return (state.unget(name), state.unget(fn), expr_stmt(state));
    }
    case lexer::Token::Kind::Identifier:
      break;
    default:
      state.unget(name);
      name = fn;
    }
    auto paren = state.get();
    state.unget(paren);
    std::unique_ptr<nodes::Expression> params;
    if (paren.kind == lexer::Token::Kind::OpenParen) {
      params = parse_expression(state);
    }
    auto brace = state.get();
    std::unique_ptr<nodes::Block> body;
    if (brace.kind == lexer::Token::Kind::OpenBrace) {
      body = block_stmt(state, brace);
    } else {
      state.unget(brace);
    }
    return std::make_unique<nodes::Fn>(fn, name, std::move(params),
                                       std::move(body));
  }

  std::unique_ptr<nodes::Statement> assign_stmt(State &state) {
    auto assignable = parse_expression(state);
    auto ctx = state.with();
    auto token = state.get();
    switch (token.kind) {
#define TK_INCLUDE TK_ASSIGN
#define TK_MACRO(_Name, _)                                                     \
  case lexer::Token::Kind::_Name:                                              \
    return std::make_unique<nodes::_Name>(token, std::move(assignable),        \
                                          expr_stmt(state));

#include <rattle/token_macro.hpp>
    default:
      state.unget(token);
      expect_eos(state);
      return assignable;
    }
  }

  std::unique_ptr<nodes::Statement> parse_statement(State &state) {
    using Kind = lexer::Token::Kind;
    // clang-format off
    while (true) {
      auto token = state.get();
      switch(token.kind) {
        case Kind::Eos:          break;
        case Kind::Error:        break;
        case Kind::HashTag:      break;
        case Kind::Eot:          return nullptr;
        case Kind::If:           return if_stmt(state, token);
        case Kind::Fn:           return fn_stmt(state, token);
        case Kind::Del:          return del_stmt(state, token);
        case Kind::Try:          return try_stmt(state, token);
        case Kind::For:          return for_stmt(state, token);
        case Kind::With:         return with_stmt(state, token);
        case Kind::Else:         return else_stmt(state, token);
        case Kind::From:         return from_stmt(state, token);
        case Kind::While:        return while_stmt(state, token);
        case Kind::OpenBrace:    return block_stmt(state, token);
        case Kind::Class:        return class_stmt(state, token);
        case Kind::Break:        return break_stmt(state, token);
        case Kind::Raise:        return raise_stmt(state, token);
        case Kind::Global:       return global_stmt(state, token);
        case Kind::Import:       return import_stmt(state, token);
        case Kind::Assert:       return assert_stmt(state, token);
        case Kind::Return:       return return_stmt(state, token);
        case Kind::Except:       return except_stmt(state, token);
        case Kind::Lastly:       return lastly_stmt(state, token);
        case Kind::NonLocal:     return nonlocal_stmt(state, token);
        case Kind::Continue:     return continue_stmt(state, token);
        case Kind::CloseParen:
          if (not state.in_paren())
            state.report(error_t::dangling_paren, token);
          break;
        case Kind::CloseBrace:
          if (not state.in_brace())
            state.report(error_t::dangling_brace, token);
          break;
        case Kind::CloseBracket:
          if (not state.in_bracket())
            state.report(error_t::dangling_bracket, token);
          break;
        default:
          state.unget(token);
          auto expr = assign_stmt(state);
          return expr.get()? std::move(expr)
            :std::make_unique<nodes::Statement>(state.get());
      }
    }
    // clang-format on
  }
} // namespace rattle::parser
