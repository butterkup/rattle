#ifndef RATTLE_SOURCE_ONLY
#include "precedence.hpp"
#include <cassert>
#include <rattle/category.hpp>
#include <rattle/parser_nodes.hpp>
#endif

namespace rattle::parser {
  prec operator--(prec const &p) { return prec(static_cast<unsigned>(p) - 1); }
  prec operator++(prec const &p) { return prec(static_cast<unsigned>(p) + 1); }

  const char *nodes::to_string(nodes::Container::Type type) {
    switch (type) {
#define TK_MACRO(Name, _)                                                      \
  case nodes::Container::Type::Name:                                           \
    return #Name;
#define TK_INCLUDE TK_CONTAINERS
#include <rattle/token_macro.hpp>
    default:
      assert(false);
    }
  }
} // namespace rattle::parser

