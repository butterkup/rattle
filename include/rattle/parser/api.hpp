#pragma once

#include "error.hpp"
#include <cstddef>
#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>

namespace rattle::parser {
  // Signals after an error occurs.
  using utility::OnError;

  struct IReactor {
    // Report an error to the reactor and move on depending on
    // the reactors signal: Abort, drain and exit; Resume, assume
    // no error happened and continue parsing.
    virtual OnError report(error::Error) noexcept = 0;
    // Allocate from say an arena or something and inplace
    // constructor will be called on the allocated memory.
    // Unfortunately, `new` and `delete` c++ mechanisms rely
    // on type info and virtual functions cannot be templated.
    // NOTE: Should return `nullptr` if no more memory is available
    // leaving the caller to handle the situation.
    virtual void *allocate(std::size_t) noexcept = 0;
    virtual ~IReactor() = default;
  };
} // namespace rattle::parser

