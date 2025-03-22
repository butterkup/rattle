#include <cassert>
#include <cctype>
#include <ostream>
#include <rattle/lexer/lexer.hpp>
#include <rattle/token/token.hpp>
#include <rattle/utility.hpp>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace rattle::lexer {
  namespace internal {
    // Mapping from keyword string to token::kinds::Identifier
    static std::unordered_map<std::string_view,
      token::kinds::Identifier::variants> const keywords{
#define rattle_pp_token_macro(kind, keyword)                                   \
  {keyword, token::kinds::Identifier::kind},
#include <rattle/token/require_pp/keywords/all.h>
#undef rattle_pp_token_macro
    };

    // consume names; keywords and variable names.
    token::Token Lexer::consume_identifier() noexcept {
      cursor.eat_while(utility::is_identifier_body_char);
      try {
        return cursor.make_token(keywords.at(cursor.buffer()));
      } catch (std::out_of_range &) {
        return cursor.make_token(token::kinds::Identifier::Variable);
      }
    }

    token::Token Lexer::consume_whitespace() noexcept {
      cursor.eat_while(utility::is_whitespace);
      return cursor.make_token(token::kinds::Marker::Whitespace);
    }

    token::Token Lexer::consume_comment() noexcept {
      cursor.eat_while([](char ch) { return ch != '\n'; });
      return cursor.make_token(token::kinds::Marker::Pound);
    }

    char Cursor::eat() noexcept {
      assert(not empty());
      current.location.column++;
      if (*current.iterator == '\n') {
        current.location.column = token::Location::Valid().column;
        reactor.cache(current.location.line++, {line_start, current.iterator});
        line_start = current.iterator + 1; // skip newline
      } else if (empty() and line_start != current.iterator) {
        reactor.cache(current.location.line, {line_start, current.iterator});
        line_start = current.iterator;
      }
      return *current.iterator++;
    }
  } // namespace internal

  namespace error {
    std::string_view to_string(Kind kind) {
      switch (kind) {
#define rattle_pp_error_macro(kind)                                            \
  case error::Kind::kind:                                                      \
    return #kind;
#include <rattle/lexer/error_pp.h>
      default:
        return "(error::Kind::Unknown)";
      }
    }

    std::ostream &operator<<(std::ostream &s, Error const &err) {
      return s << "Error{ kind=" << to_string(err.kind)
               << ", start=" << err.start << ", end=" << err.end
               << ", payload='" << utility::escape{err.lexeme} << "' }";
    }
  } // namespace error
} // namespace rattle::lexer
