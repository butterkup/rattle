#pragma once

#include "category.hpp"
#include "lexer.hpp"
#include "parser_nodes.hpp"
#include <deque>
#include <memory>
#include <vector>

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
            std::deque<Error> &errors);
      State(Lexer &lexer, std::deque<lexer::Token> &stash,
            std::deque<Error> &errors, State const &state);
      lexer::Token get(bool ignore_eos = false, bool ignore_comments = true);
      void unget(lexer::Token const &);
      bool empty() const;
      void report(error_t error, lexer::Location const &start,
                  lexer::Location const &end);
      void report(error_t error, lexer::Token const &start,
                  lexer::Token const &end);
      void report(error_t error, lexer::Token const &token);
    };
  } // namespace parser

  class Parser {
    Lexer lexer;
    std::deque<lexer::Token> stash;
    parser::State state;

  public:
    std::deque<parser::Error> errors;

    Parser();
    Parser(Parser const &parser);
    Parser(Parser &&parser);
    Parser &operator=(Parser &&parser);
    Parser &operator=(Parser const &parser);

    std::vector<std::unique_ptr<parser::nodes::Statement>> parse();
    Lexer reset(Lexer lexer_ = Lexer());
  };
} // namespace rattle

