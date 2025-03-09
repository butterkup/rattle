#pragma once

#include <rattle/ast/nodes.hpp>
#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>

namespace rattle {
  // Simply an interface to an object that converts from
  // parse tree to AST reporting any syntax errors on the way.
  struct ISyntaxAnalyzer {
    // Given a tree source offscreen, analyze the next node and return an AST
    // node of the same meaning although unnecessary details are
    // stripped and any semantic errors reported.
    [[nodiscard("We don't want to discard AST nodes, do we?")]]
    virtual utility::Scoped<ast::Stmt> analyze() = 0;
    virtual bool empty() const noexcept = 0;
    virtual void drain() noexcept {
      // Default drainage mechanism may not be efficient
      // but it definitely gets the job done.
      while (not empty()) {
        static_cast<void>(analyze()); // We do!
      }
    }
    virtual ~ISyntaxAnalyzer() = default;
  };

  // Scans the ast to ensure it makes sense semantically
  // while reporting semantic errors in the process; including but
  // not limited to proper use of statements like `return`, `break`
  // and `continue`, `nonlocal` also has semantic requirement, and
  // many more as well as variable resolution.
  struct ISemanticAnalyzer {
    virtual ~ISemanticAnalyzer() = default;
  };
} // namespace rattle

