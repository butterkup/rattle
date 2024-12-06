#ifndef RATTLE_SOURCE_ONLY
#include <rattle/analyzer.hpp>
#include <rattle/parser_nodes.hpp>
#endif

namespace rattle::analyzer {
  const char *to_string(node_t type) {
    switch (type) {
#define TK_MACRO(Name, _)                                                      \
  case node_t::Name:                                                           \
    return #Name;
#define TK_INCLUDE TK_ALL_NODES
#include <rattle/token_macro.hpp>
    }
  }

  node_t NodeType::get_type(parser::nodes::Statement &expr) {
    return (expr.visit(*this), type);
  }

#define CREATE_VISIT(_Base, _Type)                                             \
  void _Base::visit(parser::nodes::_Type &node) { type = node_t::_Type; }

#define TK_MACRO(Name, _) CREATE_VISIT(NodeType, Name)
#define TK_INCLUDE TK_ALL_NODES
#include <rattle/token_macro.hpp>
} // namespace rattle::analyzer

