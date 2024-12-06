#ifndef RATTLE_SOURCE_ONLY
#include "precedence.hpp"
#endif

namespace rattle::parser {
  prec operator--(prec const &p) { return prec(static_cast<unsigned>(p) - 1); }
  prec operator++(prec const &p) { return prec(static_cast<unsigned>(p) + 1); }
} // namespace rattle::parser

