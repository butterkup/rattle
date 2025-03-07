#pragma once

#include <rattle/ast/nodes.hpp>
#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>

namespace rattle {
  // Simply an interface to an object that converts from
  // parse tree to AST reporting any semantic errors on the way.
  struct IAnalyzer {
    // Better name could be `convert`.
    // Given a parse tree node, analyze the node and return an AST
    // node of the same meaning although unnecessary details are
    // stripped and any semantic errors reported.
    [[nodiscard("We don't want to discard AST nodes, do we?")]]
    virtual utility::Scoped<ast::Stmt> analyze(utility::Scoped<tree::Stmt>) = 0;
    virtual ~IAnalyzer() = default;
  };
} // namespace rattle

