#pragma once

#include <rattle/token/token.hpp>
#include <ostream>

namespace rattle::parser::error {
  // Error variants
  enum class Kind {
#define rattle_pp_error_macro(error) error,
#include "error_pp.h"
  };

  // Error occurred on some token as the parser sees it
  struct Error {
    Kind kind;
    token::Token token;
    Error(Kind kind, token::Token token): kind{kind}, token{token} {}
  };

  // A way to stringify the above concepts
  std::string_view to_string(Kind);
  std::ostream operator<<(std::ostream &, Error const &);
} // namespace rattle::parser::error

