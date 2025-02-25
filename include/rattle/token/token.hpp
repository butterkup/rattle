#pragma once

#include <ostream>
#include <string_view>

namespace rattle::token {
  // Track location as we see it in editors
  struct Location {
    using type = unsigned int;
    type line, column;

    bool is_null() const noexcept { return line == 0; }
    // Contructors instead of magic numbers all over the place.
    // Fabricated locations will have line number value of zero
    // while valid ones have it as non zero. Sometimes it is
    // useful to fabricate tokens, for example, testing the parser.
    static constexpr Location Valid() noexcept { return {1, 0}; }
    static constexpr Location Null() noexcept { return {0, 0}; }

    // A fabricated location is everywhere in the program
    // while no where at all; comparison to a null is always true.
    bool operator==(Location loc) noexcept {
      return is_null() || loc.is_null() ?
               true :
               line == loc.line && column == loc.column;
    }
  };

  // Preprocess token variants
  enum class Kind {
#define rattle_undef_token_macro
#define rattle_pp_token_macro(kind, _) kind,
#include "token_pp.h"
  };

  // Represent a single single token; smallest part of the program.
  struct Token {
    Kind kind;
    Location start, end;
    std::string_view lexeme;
    constexpr Token(Kind kind, Location start, Location end, std::string_view lexeme)
      : kind{kind}, start{start}, end{end}, lexeme{lexeme} {}
  };
  // Return string representation of the token variant
  std::string_view to_string(Kind) noexcept;

  // Printers
  std::ostream &operator<<(std::ostream &, Location) noexcept;
  std::ostream &operator<<(std::ostream &, Token const &) noexcept;
} // namespace rattle::token

