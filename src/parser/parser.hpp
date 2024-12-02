#pragma once

#include <memory>
#include <rattle/parser.hpp>
#include <rattle/parser_nodes.hpp>

namespace rattle::parser {
  std::unique_ptr<nodes::Expression> parse_expression(State &state);
  std::unique_ptr<nodes::Statement> parse_statement(State &state);
} // namespace rattle::parser

