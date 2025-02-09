#pragma once

#include <cassert>
#include <cstddef>
#include <string_view>

namespace rattle::lexer {
  // Track location as we see it in editors
  struct location_t {
    using type = unsigned int;
    type line, column;

    bool is_null() const { return line == 0; }
    static constexpr location_t valid() { return {1, 0}; }
    static constexpr location_t null() { return {0, 0}; }
  };

  // Preprocess error variants
  enum class error_kind_t {
#define ERROR_MACRO(error) error,
#include "errors_pp.h"
  };

  // Return string representation of the error variant
  std::string_view to_string(error_kind_t);

  // Represent an error from the lexer phase.
  struct error_t {
    error_kind_t kind;
    location_t start, end;
    std::string_view lexeme;

    error_t(error_kind_t kind, location_t start, location_t end,
      std::string_view lexeme)
      : kind{kind}, start{start}, end{end}, lexeme{lexeme} {}
  };

  // Preprocess token variants
  enum class token_kind_t {
#define rattle_undef_forget_token_macro
#define TOKEN_MACRO(kind, _) kind,
#include "tokens_pp.h"
  };

  // Return string representation of the token variant
  std::string_view to_string(token_kind_t);

  // Represent a single single token; smallest part of the program.
  struct token_t {
    token_kind_t kind;
    location_t start, end;
    std::string_view lexeme;
    token_t(token_kind_t kind, location_t start, location_t end,
      std::string_view lexeme)
      : kind{kind}, start{start}, end{end}, lexeme{lexeme} {}
  };

  // Very useful for printing strings with control characters.
  // Simply a type tag over which std::string_view printer to use:
  // ours for escaping special characters (control, etc); the one provided
  // by standard library, possible impl would be a wrapper over
  // `std::ostream::write(ptr, len)`
  struct escape_t {
    std::string_view view;
  };

  // Printers: token_t error_t location_t escape_t
  std::ostream &operator<<(std::ostream &, token_t const &);
  std::ostream &operator<<(std::ostream &, error_t const &);
  std::ostream &operator<<(std::ostream &, location_t const &);
  std::ostream &operator<<(std::ostream &, escape_t const &);

  // No need to compute some things twice.
  // * Lines will be needed to report errors, why not cache them here.
  // * The internal state of the lexer needn't care how errors flow
  //   in the system, only that it reports and goes on.
  struct manager_t {
    enum class onerror { short_circuit, keep_going };
    virtual onerror report_error(error_t) = 0;
    virtual void cache_line(std::size_t, const std::string_view) = 0;
    virtual ~manager_t() = default;
  };

  namespace internal {
    // Represents a snapshot of where we are.
    struct state_t {
      location_t location;
      std::string_view::const_iterator iterator;
    };
    // Cursor base, provides common operations while abstracting
    // low level iterator gymnastics
    class cursor_t {
      const std::string_view program;
      state_t start, current;
      std::string_view::const_iterator line_start;
      manager_t &manager;

      void flush_program() {
        start.iterator = current.iterator = program.cend();
      }

    public:
      cursor_t(const std::string_view program, manager_t &manager)
        : program{program}, start{location_t::valid(), program.cbegin()},
          current{location_t::valid(), program.cbegin()},
          line_start{program.cbegin()}, manager{manager} {}

      // Check if we are at the end of the program.
      bool empty() const { return current.iterator == program.cend(); }
      // Preconditions, nth char exists
      char peek(std::ptrdiff_t n = 0) const {
        assert(safe(n));
        return *(current.iterator + n);
      }
      // Report an error to the manager
      void report(error_t error) {
        if (manager.report_error(error) == manager_t::onerror::short_circuit) {
          flush_program();
        } /* else keep_going */
      }
      void report(error_kind_t error) {
        report({error, start.location, current.location, get_lexeme()});
      }
      void report(error_kind_t error, state_t mark) {
        report({error, mark.location, current.location,
          {mark.iterator, current.iterator}});
      }
      // Flush lexeme, note: the current character is not yet consumed
      void consume_lexeme() { start = current; }
      // Get location where lexeme started
      location_t start_location() const { return start.location; }
      // Get current location; where we at?
      location_t current_location() const { return current.location; }
      // Get current lexeme collected so far
      std::string_view get_lexeme() const {
        return {start.iterator, current.iterator};
      }
      // Get current point in the program
      state_t bookmark() const { return current; }
      // Consume a character appropriately. [UNSAFE]
      // precondition: cursor has not reached end of file
      char eat();
      // Check how far ahead we can peek safely
      std::ptrdiff_t max_safe() const {
        return program.cend() - current.iterator;
      }
      // Can we peek n characters ahead, default 0 which checks if current character
      // is safe to peek
      bool safe(std::ptrdiff_t n = 0) const { return n < max_safe(); }
      // Advance and return true if current character is what we expect
      // otherwise return false
      bool match(char expected) {
        return safe() and expected == peek() ? (eat(), true) : false;
      }
      // Advance and match if safe
      bool match_next(char expected) { return (eat(), match(expected)); }
      // Make a token of kind
      token_t make_token(token_kind_t kind) {
        token_t token = {kind, start.location, current.location, get_lexeme()};
        consume_lexeme();
        return token;
      }
      // Report and make an error token
      token_t make_token(error_kind_t kind) {
        report(kind);
        return make_token(token_kind_t::Error);
      }
      // Consume while some predicate is true
      template <class predicate_t>
      std::size_t eat_while(predicate_t &&predicate) {
        std::size_t consumed = 0;
        while (not empty() and predicate(peek())) {
          consumed++;
          eat();
        }
        return consumed;
      }
    };
    // Actual lexer, moves through the program creating tokens.
    // It tracks two states, where a token starts and where we are
    // currently and notifies the manager on relevant events like
    // errors encountered and lines processed.
    struct lexer_t {
      lexer_t(std::string_view program, manager_t &manager)
        : base{program, manager} {}
      // Scan the next token
      token_t scan();
      bool empty() const { return base.empty(); }

    private:
      // manages position in the program (aka actual/core cursor)
      cursor_t base;
      // They do as they say and wrap as a token
      token_t consume_whitespace();
      token_t consume_comment();
      token_t consume_string();
      token_t consume_number();
      token_t consume_identifier();
    };
  } // namespace internal

  // A facade, simpler and hides the complexity by exposing a limited
  // API to its owner, afterall, they mainly expect `lexer.lex() -> token`
  struct lexer_t {
    lexer_t(std::string_view program, manager_t &manager)
      : lexer{program, manager} {}
    token_t lex() { return lexer.scan(); }
    bool empty() const { return lexer.empty(); }

  private:
    internal::lexer_t lexer;
  };
} // namespace rattle::lexer
