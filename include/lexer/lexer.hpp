#include <string_view>

namespace rattle::lexer {
  // Track location as we see it in editors
  struct location_t {
    std::size_t line, column;
  };

  // Preprocess error variants
  enum class error_kind_t {
#define ERROR_MACRO(error) error,
#include "errors_pp.h"
  };

  // Represent an error from the lexer phase.
  struct error_t {
    error_kind_t kind;
    location_t start, end;
    const std::string_view lexeme;

    error_t(error_kind_t kind, location_t start, location_t end,
            const std::string_view lexeme)
      : kind{kind}, start{start}, end{end}, lexeme{lexeme} {}
  };

  // Preprocess token variants
  enum class token_kind_t {
#define TOKEN_MACRO(kind, _) kind,
#include "tokens_pp.h"
  };

  // Represent a single single token; smallest part of the program.
  struct token_t {
    token_kind_t kind;
    location_t start, end;
    const std::string_view lexeme;
    token_t(token_kind_t kind, location_t start, location_t end,
            const std::string_view lexeme)
      : kind{kind}, start{start}, end{end}, lexeme{lexeme} {}
  };

  // No need to compute some things twice.
  // * Lines will be needed to report errors, why not cache them here.
  // * The internal state of the lexer needn't care how errors flow
  //   in the system, only that it reports and goes on.
  struct manager_t {
    virtual void report_error(error_t error) = 0;
    virtual void cache_line(const std::string_view line) = 0;
  };

  namespace internal {
    // Represents a snapshot of where we are.
    struct real_state_t {
      location_t location;
      std::string_view::const_iterator iterator;
    };

    // Actual lexer, moves through the program creating tokens.
    // It tracks two states, where a token starts and where we are
    // currently and notifies the manager on relevant events like
    // errors encountered and lines processed.
    struct state_t {
      const std::string_view program;
      real_state_t start, current;
      std::string_view::const_iterator line_start;
      manager_t &manager;

      state_t(const std::string_view program, manager_t &control)
        : program{program}, start{location_t{1, 1}, program.cbegin()},
          current{location_t{1, 1}, program.cbegin()},
          line_start{program.cbegin()}, manager{control} {}

      // Check if we are at the end of the program.
      bool empty() const { return current.iterator == program.cend(); }
    };
  } // namespace internal

  // A facade, simpler and hides the complexity by exposing a limited
  // API to its owner, afterall, they mainly expect `lexer.lex() -> token`
  struct lexer_t {
    lexer_t(const std::string_view program, manager_t &manager)
      : state{program, manager} {}
    token_t lex();

  private:
    internal::state_t state;
  };
} // namespace rattle::lexer
