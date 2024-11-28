#pragma once

#include <memory>
#include <rattle/node.hpp>
#include <rattle/parser.hpp>

namespace rattle::parser {
  std::unique_ptr<nodes::Expression> parse_expression(State &state);
}

