#include <memory>
#include <rattle/tree/nodes.hpp>

namespace rattle {
  // Interface for Parser type
  struct IParser {
    // Should return an empty `unique_ptr` if parser
    // is empty, otherwise well formed nodes are expected
    virtual std::unique_ptr<tree::Stmt> parse() = 0;
    // Can the parser produce more nodes?
    virtual bool empty() const = 0;
    // Discard all nodes till it is empty
    virtual void drain() {
      // The Concrete implementation may override for faster
      // more efficient draining, as a default, it gets the
      // job done.
      while (not empty()) {
        parse();
      }
    }
    virtual ~IParser() = default;
  };
} // namespace rattle

