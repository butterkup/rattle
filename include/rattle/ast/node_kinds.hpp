#pragma once

#include <rattle/tree/nodes.hpp>
#include <rattle/utility.hpp>

namespace rattle::ast::kinds {
  using utility::Scoped;

  enum class Def { Function };
  enum class Binding { Name, Capture };
  enum class Sequence { Tuple, List };
  enum class Command { Nonlocal, Global, Return };
  enum class Event { ScopeBegin, ScopeEnd, Continue, Break };

#define rattle_pp_token_macro(Kind, _) Kind,

  enum class Assignment {
#include "require_pp/assignment.h"
  };

  enum class Literal {
#include "require_pp/expression/literal.h"
  };

  enum class BinaryExpr {
#include "require_pp/expression/binary.h"
  };

  enum class UnaryExpr {
#include "require_pp/expression/unary.h"
  };

  enum class TernaryExpr {
#include "require_pp/expression/ternary.h"
  };

#undef rattle_pp_token_macro
} // namespace rattle::ast::kinds

