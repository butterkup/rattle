#pragma once

#include <rattle/token/token.hpp>

namespace rattle::analyzer::syntax::error {
  // syntax errors are represented by this class
  // not the best for now but a working draft
  struct Error {
    const char *description;
    token::Location begin;
    token::Location end;
  };
} // namespace rattle::analyzer::syntax::error

