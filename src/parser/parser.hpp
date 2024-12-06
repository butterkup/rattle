#pragma once

#ifndef RATTLE_SOURCE_ONLY
#include <memory>
#include <rattle/parser.hpp>
#include <rattle/parser_nodes.hpp>
#endif

namespace rattle::parser {
  std::unique_ptr<nodes::Expression> parse_expression(State &state);
  std::unique_ptr<nodes::Statement> parse_statement(State &state);
} // namespace rattle::parser

