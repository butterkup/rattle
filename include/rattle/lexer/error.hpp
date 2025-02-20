#pragma once

#include <rattle/token/token.hpp>
#include <string_view>

namespace rattle::lexer::error {
  // Preprocess error variants.
  // What kind of error occurred?
  enum class Kind {
#define rattle_pp_error_macro(error) error,
#include "error_pp.h"
  };

  // Return string representation of the error variant
  std::string_view to_string(Kind);

  // Represent an error from the lexer phase.
  struct Error {
    Kind kind;
    token::Location start, end;
    std::string_view lexeme;

    Error(Kind kind, token::Location start, token::Location end,
      std::string_view lexeme)
      : kind{kind}, start{start}, end{end}, lexeme{lexeme} {}
  };

  // Printer
  std::ostream &operator<<(std::ostream &, Error const &);
} // namespace rattle::lexer::error
