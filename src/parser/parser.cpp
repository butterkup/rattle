#include "rattle/lexer.hpp"
#include <memory>
#include <rattle/parser_nodes.hpp>
#include <rattle/parser.hpp>
#include <utility>

namespace rattle {
  std::unique_ptr<parser::nodes::Statement> Parser::parse() { return {}; }
  Lexer Parser::reset(Lexer lexer_) {
    Lexer tmp = std::move(lexer);
    lexer = std::move(lexer_);
    return tmp;
  }
  Parser::Parser(): lexer(), stash(), state(lexer, stash, errors), errors() {}
  Parser::Parser(Lexer lexer)
    : lexer(std::move(lexer)), stash(), state(lexer, stash, errors), errors() {}
  Parser::Parser(Parser const &parser)
    : lexer(parser.lexer), stash(parser.stash),
      state(lexer, stash, errors, parser.state), errors(parser.errors) {}
  Parser::Parser(Parser &&parser)
    : lexer(std::move(parser.lexer)), stash(std::move(parser.stash)),
      state(lexer, stash, errors, parser.state),
      errors(std::move(parser.errors)) {}
  Parser &Parser::operator=(Parser &&parser) {
    ::new (this) Parser(std::move(parser));
    return *this;
  }
  Parser &Parser::operator=(Parser const &parser) {
    ::new (this) Parser(parser);
    return *this;
  }
} // namespace rattle

