#pragma once

#include "error.hpp"
#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>

namespace rattle::parser {
  using utility::OnError;

  struct IReactor {
    virtual OnError report(error::Error) = 0;
  };
} // namespace rattle::parser

