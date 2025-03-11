#pragma once

#include "error.hpp"
#include <cstddef>
#include <rattle/utility.hpp>

namespace rattle::analyzer::syntax {
  struct IReactor {
    virtual void *allocate(std::size_t) noexcept = 0;
    virtual utility::OnError report(utility::Scoped<error::Error>) noexcept = 0;
  };
} // namespace rattle::analyzer

