#pragma once

#include "token_category.hpp"
#include "utility.hpp"
#include <ostream>
#include <string_view>
#include <unordered_map>

namespace rattle::token {
  namespace __detail {
    enum class token_kind {
#define TK_MACRO(kind, _) kind,
#include "token_macro.hpp"
    };
    std::ostream &operator<<(std::ostream &out, token_kind const &tk);
  } // namespace __detail

  struct Token {
    using Kind = __detail::token_kind;
    Kind kind;
    Location start, end;
  };
  std::ostream &operator<<(std::ostream &out, Token const &tk);

  const std::unordered_map<std::string_view, Token::Kind> keywords = {
#define TK_MACRO(kind, strrep) {strrep, Token::Kind::kind},
#define TK_INCLUDE TK_KEYWORD
#include "token_macro.hpp"
  };
} // namespace rattle::token
