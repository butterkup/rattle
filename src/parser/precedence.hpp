#pragma once

#include <cassert>

namespace rattle::parser::internal::expr {
  enum class Prec : unsigned {
    none,   // Sentinel for no precedence
    lowest, // Sentinel for lowest precedence; marker value, do not use.

    comma,                         // `a, b`
    if_else,                      // `a if b else c`
    logic_or,                     // `a or b`
    logic_and,                    // `a and b`
    logic_not,                    // `not a`
    compare_eq,                   // `a == b`
    compare_ne = compare_eq,      // `a != b`
    compare,                      // `a < b` `a > b` `a <= b` `a >= b`
    bitwise_or,                   // `a | b`
    bitwise_and,                  // `a & b`
    identity_is,                  // `a is b` `a is not b`
    member_in,                    // `a in seq` `a not in seq`
    add,                          // `a + b`
    subtract,                     // `a - b`
    multiply,                     // `a * b`
    divide,                       // `a / b`
    negate,                       // `-a`
    posify = negate,              // `+a`
    spread = negate,              // `*a`
    dot,                          // `a.b`
    call = dot,                   // `a(b)`
    subscript = dot,              // `a[b]`
    lambda,                       // `|a, b| a + b`
    group,                        // `(expr)`
    primary, // all literals; identifier, number, string, etc.

    highest, // Sentinel for highest precedence; marker value, do not use.
  };

  inline Prec operator++(Prec p) {
    assert(p != Prec::highest);
    return static_cast<Prec>(static_cast<unsigned>(p) + 1);
  }

  inline Prec operator--(Prec p) {
    assert(p != Prec::lowest);
    return static_cast<Prec>(static_cast<unsigned>(p) - 1);
  }
} // namespace rattle::parser::internal::expr

