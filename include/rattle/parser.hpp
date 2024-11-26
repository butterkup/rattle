#pragma once

#include "category.hpp"
#include "lexer.hpp"
#include "rattle/node.hpp"
#include <deque>
#include <memory>

namespace rattle {
  namespace parser {
    enum class error_t {
#define ERROR_MACRO(error) error,
#define ERROR_INCLUDE PARSER_ERROR
#include "error_macro.hpp"
    };

    struct Error {
      error_t type;
      lexer::Location start, end;

      Error(error_t type, lexer::Location const &start,
            lexer::Location const &end)
        : type(type), start(start), end(end) {}
    };

    class State {
      Lexer &lexer;
      std::deque<lexer::Token> &stash;
      std::deque<Error> &errors;
      bool hit_eot;

    public:
      State(State &&) = delete;
      State(State const &) = delete;
      State(Lexer &lexer, std::deque<lexer::Token> &stash,
            std::deque<Error> &errors)
        : lexer(lexer), stash(stash), errors(errors), hit_eot(false) {}
      State(Lexer &lexer, std::deque<lexer::Token> &stash,
            std::deque<Error> &errors, State const &state)
        : lexer(lexer), stash(stash), errors(errors), hit_eot(state.hit_eot) {}

      lexer::Token get(bool ignore_comments = true);
      void unget(lexer::Token const &);
      bool empty() const;
      void report(error_t error, lexer::Location const &start,
                  lexer::Location const &end);
      void report(error_t error, lexer::Location const &start);
      void report(error_t error);
    };
  } // namespace parser

  class Parser {
    Lexer lexer;
    std::deque<lexer::Token> stash;
    parser::State state;

  public:
    std::deque<parser::Error> errors;

    Parser(): lexer(), stash(), state(lexer, stash, errors), errors() {}
    Parser(Lexer lexer)
      : lexer(std::move(lexer)), stash(), state(lexer, stash, errors),
        errors() {}
    Parser(Parser const &parser)
      : lexer(parser.lexer), stash(parser.stash),
        state(lexer, stash, errors, parser.state), errors(parser.errors) {}
    Parser(Parser &&parser)
      : lexer(std::move(parser.lexer)), stash(std::move(parser.stash)),
        state(lexer, stash, errors, parser.state),
        errors(std::move(parser.errors)) {}

    std::unique_ptr<parser::nodes::Statement> parse();
    Lexer reset(Lexer lexer_ = Lexer());
  };
} // namespace rattle

