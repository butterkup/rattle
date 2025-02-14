#pragma once

#include "error.hpp"
#include <cstddef>
#include <rattle/token/token.hpp>
#include <rattle/utility.hpp>
#include <string_view>

namespace rattle::lexer {
  // How the lexer should behave after an error occurred.
  // * Abort - drain the lexer
  // * Resume - ignore the error and keep going
  using utility::OnError;
  // No need to compute some things twice.
  // * Lines will be needed to report errors, why not cache them here.
  // * The internal state of the lexer needn't care how errors flow
  //   in the system, only that it reports and goes on.
  struct IReactor {
    // Reporting an error is the most important and must be provided
    // the others can be ignored but tappable.
    virtual OnError report(error::Error) = 0;
    // Notify the reactor when a whole line is consumed.
    // Reports  the line number and line contents.
    // NOTE: Line contents will not include the newline character at the end
    // for consistency
    virtual void cache(std::size_t, std::string_view) {};
    // Notify the reactor when a token is created
    // For debugging or something!!! Do as You Desire.
    virtual void trace(token::Token &) {}
    virtual ~IReactor() = default;
  };
} // namespace rattle::lexer

