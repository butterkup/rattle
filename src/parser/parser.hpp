#pragma once

#include <memory>
#include <rattle/parser_nodes.hpp>
#include <rattle/parser.hpp>

namespace rattle::parser {
  std::unique_ptr<nodes::Expression> parse_expression(State &state);
  lexer::Token get(State &state, bool ignore_eos=true, bool ignore_comments=true);
}

