#pragma once

namespace rattle::parser {
  enum class prec : unsigned {
    none,              // All other tokens
    _lowest,           // Sentinel for lowest precedence, don't use.
    as,                // some.expr.here as name
    yield,             // yield <expr>
    comma,             // a , b
    colon,             // a : b
    logic_or,          // a or b
    logic_and,         // a and b
    logic_not,         // not a
    equal,             // a == b
    not_equal = equal, // a != b
    compare,           // a>b a>=b a<b a<=b
    shift,             // a<<b  a>>b
    bit_or,            // a | b
    xor_,              // a ^ b
    bit_and,           // a & b
    is,                // a is b      a is not b
    in,                // a in seq    a not in seq
    plus,              // a + b
    minus = plus,      // a - b
    matmul,            // a @ b
    multiply = matmul, // a * b
    divide,            // a / b
    modulus = divide,  // a % b
    uplus,             // +a
    uminus = uplus,    // -a
    invert = uplus,    // ~a
    subscript,         // a[b]
    call,              // a()
    dot,               // a.b
    primary,           // All Litarals
    _highest,          // Sentinel for highest precedence, don't use.
  };

  prec operator--(prec const &p);
  prec operator++(prec const &p);
} // namespace rattle::parser

