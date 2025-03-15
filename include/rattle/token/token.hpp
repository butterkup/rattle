#pragma once

#include <ostream>
#include <string_view>

// Very useful for cheking if a token is a very specific
// kind taking the flags into consideration which always
// yield unique variants
#define rattle_merge_kind(TokenKind, Flags)                                    \
  ((static_cast<std::size_t>(TokenKind) << 32) | Flags)

// Save a few keystrokes specifying the namespace path
#define rattle_merge_kind2(Kind, Flag)                                         \
  rattle_merge_kind(                                                           \
    rattle::token::kinds::Token::Kind, rattle::token::kinds::Kind::Flag)
#define rattle_merge_kind3(Kind, Flag1, Flag2)                                 \
  (rattle_merge_kind2(Kind, Flag1) | rattle_merge_kind2(Kind, Flag2))

namespace rattle::token {
  namespace kinds {
    // NOTE: The weird nested enums inside structs help contain the enum
    // variants in the struct scopes, could have used enum class but then
    // we would have to add operator overloading, even assigning to an int
    // would need a static_cast, avoid all that and just contain the enums.

    struct Identifier {
      enum variants {
#define rattle_pp_token_macro(Ident, _) Ident,
#include "require_pp/kinds/identifier.h"
#undef rattle_pp_token_macro
      };
    };

    struct Operator {
      enum variants {
#define rattle_pp_token_macro(Op, _) Op,
#include "require_pp/kinds/operator.h"
#undef rattle_pp_token_macro
      };
    };

    struct String {
      enum variants {
#define rattle_pp_token_macro(Flag, Value) Flag = Value,
#include "require_pp/kinds/string.h"
#undef rattle_pp_token_macro
      };
    };

    struct Number {
      enum variants {
#define rattle_pp_token_macro(Kind, Value) Kind = Value,
#include "require_pp/kinds/number.h"
#undef rattle_pp_token_macro
      };
    };

    struct Assignment {
      enum variants {
#define rattle_pp_token_macro(Kind, _) Kind,
#include "require_pp/kinds/assignment.h"
#undef rattle_pp_token_macro
      };
    };

    struct Marker {
      enum variants {
#define rattle_pp_token_macro(Kind, _) Kind,
#include "require_pp/kinds/marker.h"
#undef rattle_pp_token_macro
      };
    };

    // Preprocess token variants
    // Unlike the ones up there, token kinds should be strongly typed
    // I believe, since they are limited and don't go into many conversions
    // and operators except switches.
    enum class Token {
#define rattle_pp_token_macro(Kind, _) Kind,
#include "require_pp/kind.h"
#undef rattle_pp_token_macro
    };

  } // namespace kinds
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

  // Represent a single single token; smallest part of the program.
  struct Token {
    // What kind of token is this?
    kinds::Token kind;
    // Some tokens fall under one umbrella so storing those variants
    // could be done with this little int here, could be wasted to
    // padding so no size increase for the Token type.
    int flags;
    // Where is the token in the input program? It runs from `start` to `end`.
    Location start, end;
    // The contents of this token.
    std::string_view lexeme;
    constexpr Token(kinds::Token kind, int flags, Location start, Location end,
      std::string_view lexeme)
      : kind{kind}, flags{flags}, start{start}, end{end}, lexeme{lexeme} {}

    // Combine both flags and kind to get a unique integer telling specifically
    // what kind of token this is.
    constexpr inline std::size_t merge() const noexcept {
      return rattle_merge_kind(kind, flags);
    }
  };

  // Return string representation of the token variant
  std::string_view to_string(kinds::Token) noexcept;

  // Printers
  std::ostream &operator<<(std::ostream &, Location) noexcept;
  std::ostream &operator<<(std::ostream &, Token const &) noexcept;
} // namespace rattle::token

