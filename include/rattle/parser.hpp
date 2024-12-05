#pragma once

#include "category.hpp"
#include "lexer.hpp"
#include "parser_nodes.hpp"
#include <cstdint>
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

    const char *to_string(error_t error);

    struct Error {
      error_t type;
      lexer::Location start, end;

      Error(error_t type, lexer::Location const &start,
            lexer::Location const &end)
        : type(type), start(start), end(end) {}
    };

    namespace __detail {
      struct ContextSetting;
      struct Scope;
      struct Scopes {
        int paren, brace, bracket;
      };
    } // namespace __detail

    class State {
      Lexer &lexer;
      std::deque<lexer::Token> &stash;
      std::deque<Error> &errors;
      std::deque<int> settings;
      bool hit_eot;
      std::uint8_t context;
      __detail::Scopes scopes;

      void push(std::uint8_t setting = DEFAULT);
      void pop();

    public:
      friend struct __detail::ContextSetting;
      friend struct __detail::Scope;
      enum : std::uint8_t {
        // Bit state settings.
        NONE = 0,
        IGNORE_COMMENTS = 2,
        IGNORE_NEWLINE = 4,
        IGNORE_SEMICOLON = 8,

        // Shorter constants, most used.
        DEFAULT = IGNORE_COMMENTS,
        IGNORE_EOS = IGNORE_NEWLINE | IGNORE_SEMICOLON,
        IGNORE_NLCOM = IGNORE_NEWLINE | IGNORE_COMMENTS,
        IGNORE_SEMCOM = IGNORE_SEMICOLON | IGNORE_COMMENTS,
        IGNORE_EOSCOM = IGNORE_EOS | IGNORE_COMMENTS,
      };
      State(State &&) = delete;
      State(State const &) = delete;
      State(Lexer &lexer, std::deque<lexer::Token> &stash,
            std::deque<Error> &errors)
        : lexer(lexer), stash(stash), errors(errors), settings(),
          hit_eot(false), context(DEFAULT), scopes(0) {}
      State(Lexer &lexer, std::deque<lexer::Token> &stash,
            std::deque<Error> &errors, State const &state)
        : lexer(lexer), stash(stash), errors(errors), settings(state.settings),
          hit_eot(state.hit_eot), context(state.context), scopes(state.scopes) {
      }

      bool test(std::uint8_t setting) const;
      bool in_paren() const;
      bool in_bracket() const;
      bool in_brace() const;
      std::uint8_t setting() const;
      bool empty() const;
      __detail::ContextSetting with(std::uint8_t setting = DEFAULT);
      __detail::Scope enter_paren();
      __detail::Scope enter_bracket();
      __detail::Scope enter_brace();
      lexer::Token get();
      void unget(lexer::Token const &);
      void report(error_t error, lexer::Location const &start,
                  lexer::Location const &end);
      void report(error_t error, lexer::Token const &start,
                  lexer::Token const &end);
      void report(error_t error, lexer::Token const &token);
    };

    namespace __detail {
      struct Scope {
        Scope(Scope &&) = delete;
        Scope(Scope const &) = delete;
        Scope(int &scope_cnt): scope_count(scope_cnt) { scope_count++; }
        ~Scope() { scope_count--; }

      private:
        int &scope_count;
      };

      struct ContextSetting {
        State &managed;
        ContextSetting(ContextSetting &&) = delete;
        ContextSetting(ContextSetting const &) = delete;
        ContextSetting(State &managed): managed(managed) {}
        ContextSetting(State &managed, std::uint8_t setting): managed(managed) {
          managed.push(setting);
        }
        ~ContextSetting() { managed.pop(); }
      };
    } // namespace __detail
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

