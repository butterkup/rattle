#include "parser.hpp"
#include "rattle/lexer.hpp"
#include <memory>
#include <rattle/parser.hpp>
#include <rattle/parser_nodes.hpp>
#include <utility>

namespace rattle {
  std::vector<std::unique_ptr<parser::nodes::Statement>> Parser::parse() {
    std::vector<std::unique_ptr<parser::nodes::Statement>> stmts;
    while (auto stmt = parser::parse_statement(state)) {
      stmts.emplace_back(std::move(stmt));
    }
    stmts.shrink_to_fit();
    return stmts;
  }

  Lexer Parser::reset(Lexer lexer_) {
    Lexer tmp = std::move(lexer);
    lexer = std::move(lexer_);
    return tmp;
  }

  const char *to_string(parser::error_t error) {
    switch (error) {
#define ERROR_MACRO(_error)                                                    \
  case parser::error_t::_error:                                                \
    return #_error;
#define ERROR_INCLUDE PARSER_ERROR
#include <rattle/error_macro.hpp>
    }
  }

  Parser::Parser(): lexer(), stash(), state(lexer, stash, errors), errors() {}
  Parser::Parser(Parser const &parser)
    : lexer(parser.lexer), stash(parser.stash),
      state(lexer, stash, errors, parser.state), errors(parser.errors) {}
  Parser::Parser(Parser &&parser)
    : lexer(std::move(parser.lexer)), stash(std::move(parser.stash)),
      state(lexer, stash, errors, parser.state),
      errors(std::move(parser.errors)) {}
  Parser &Parser::operator=(Parser &&parser) {
    this->~Parser();
    ::new (this) Parser(std::move(parser));
    return *this;
  }
  Parser &Parser::operator=(Parser const &parser) {
    this->~Parser();
    ::new (this) Parser(parser);
    return *this;
  }
} // namespace rattle

