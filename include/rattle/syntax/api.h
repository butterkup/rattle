#pragma once

#include "error.hpp"
#include <cstddef>
#include <rattle/utility.hpp>

namespace rattle::analyzer::syntax {
  struct IReactor {
    virtual void *allocate(std::size_t) noexcept = 0;
    virtual utility::OnError report(error::Error const &) noexcept = 0;
  };
} // namespace rattle::analyzer::syntax

