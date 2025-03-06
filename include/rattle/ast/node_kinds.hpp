#pragma once

#include "rattle/utility.hpp"
#include <rattle/tree/nodes.hpp>

namespace rattle::ast::kinds {
  using utility::Scoped;

  enum class Binding { Name, Capture };
  enum class Sequence { Tuple, List };

#define rattle_pp_token_macro(Kind, _) Kind,
  enum class Assignment {
#include "require_pp/assignment.h"
#include "require_pp/undefine.h"
  };

  enum class Literal {
#include "require_pp/expression/literal.h"
#include "require_pp/undefine.h"
  };

  enum class BinaryExpr {
#include "require_pp/expression/binary.h"
#include "require_pp/undefine.h"
  };

  enum class UnaryExpr {
#include "require_pp/expression/unary.h"
#include "require_pp/undefine.h"
  };

  enum class TernaryExpr {
#include "require_pp/expression/ternary.h"
#include "require_pp/undefine.h"
  };
#define rattle_undef_token_macro
#include "require_pp/undefine.h"
} // namespace rattle::ast::kinds

