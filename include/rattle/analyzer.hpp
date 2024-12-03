#pragma once

#include <rattle/parser_nodes.hpp>

namespace rattle::analyzer {
  enum class node_t {
#define TK_MACRO(Name, _) Name,
#define TK_INCLUDE TK_ALL_NODES
#include <rattle/token_macro.hpp>
  };

  const char *to_string(node_t type);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#define CREATE_VISIT(_Type) void visit(parser::nodes::_Type &node) override;

  struct NodeType: parser::nodes::Visitor {
    NodeType(): type(node_t::Statement) {}
    node_t get_type(parser::nodes::Statement &node);

#define TK_INCLUDE TK_ALL_NODES
#define TK_MACRO(Name, _) CREATE_VISIT(Name)
#include <rattle/token_macro.hpp>
  private:
    node_t type;
  };

#undef CREATE_VISIT

#pragma GCC diagnostic pop
} // namespace rattle::analyzer

