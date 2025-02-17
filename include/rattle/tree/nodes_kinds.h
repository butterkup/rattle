#pragma once

#define rattle_pp_token_macro(Kind, _) Kind,

namespace rattle::tree::kinds {
  enum class Node {
    None,
    Expression,
    Statement,
  };
  enum class Assignment {
#include "require_pp/assignment.h"
#include "require_pp/undefine.h"
  };

  enum class Literal {
#include "require_pp/literal.h"
#include "require_pp/undefine.h"
  };

  enum class Expression {
#include "require_pp/expression.h"
#include "require_pp/undefine.h"
  };
} // namespace rattle::tree::kinds

#define rattle_undef_token_macro
#include "require_pp/undefine.h"
