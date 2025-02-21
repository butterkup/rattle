#include <cassert>
#include <cctype>
#include <ostream>
#include <rattle/lexer/lexer.hpp>
#include <rattle/utility.hpp>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace rattle::lexer {
  namespace internal {
    // Mapping from keyword string to token::Kind
    static std::unordered_map<std::string_view, token::Kind> const keywords{
#define rattle_undef_token_macro
#define rattle_pp_token_macro(kind, keyword) {keyword, token::Kind::kind},
#include <rattle/token/require_pp/keywords.h>
#include <rattle/token/require_pp/undefine.h>
    };

    // consume names; keywords and variable names.
    token::Token Lexer::consume_identifier() noexcept {
      base.eat_while(utility::is_identifier_body_char);
      try {
        return base.make_token(keywords.at(base.buffer()));
      } catch (std::out_of_range &) {
        return base.make_token(token::Kind::Identifier);
      }
    }

    token::Token Lexer::consume_whitespace() noexcept {
      base.eat_while(utility::is_whitespace);
      return base.make_token(token::Kind::Whitespace);
    }

    token::Token Lexer::consume_comment() noexcept {
      base.eat_while([](char ch) { return ch != '\n'; });
      return base.make_token(token::Kind::Pound);
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
