#pragma once

#include <utility>
namespace rattle::parser {
  enum class prec : unsigned {
    _lowest,   // Sentinel for lowest precedence, don't use.
    none,      // All other tokens
    group,     // (a)
    subscript, // a[b]
    dot,       // a.b
    call,      // a()
    matmul,    // a @ b
    comma,     // Separator(Not an operator): a, b
    multiply,  // a * b
    divide,    // a / b
    modulus,   // a % b
    uplus,     // +a
    uminus,    // -a
    plus,      // a + b
    minus,     // a - b
    compare,   // a>b a>=b a<b a<=b
    equal,     // a == b
    not_equal, // a != b
    shift,     // a<<b  a>>b
    bit_and,   // a & b
    bit_or,    // a | b
    logic_and, // a and b
    logic_or,  // a or b
    logic_not, // not a
    invert,    // ~a
    is,        // a is b
    in,        // a in seq
    primary,   // True, None, False, Numbers, Strings and Variables
    _highest,  // Sentinel for highest precedence, don't use.
  };
} // namespace rattle::parser

