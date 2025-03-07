#pragma once

#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>

namespace rattle {
  // Interface for Parser type
  struct IParser {
    // Should return an empty `Scoped` if parser
    // is empty, otherwise well formed nodes are expected
    // Parse a statement
    [[nodiscard("We don't want to leak nodes, do we?")]]
    virtual utility::Scoped<tree::Stmt> parse() noexcept = 0;
    // Can the parser produce more nodes? If token source is not empty
    // more nodes are possible.
    virtual bool empty() const noexcept = 0;
    // Discard all nodes till it is empty
    virtual void drain() noexcept {
      // The Concrete implementation may override for faster
      // more efficient draining, as a default, it gets the
      // job done.
      while (not empty()) {
        // We do!
        static_cast<void>(parse());
      }
    }
    virtual ~IParser() = default;
  };
} // namespace rattle

