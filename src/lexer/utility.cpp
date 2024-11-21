#include "lexer.hpp"
#include <rattle/category.hpp>
#include <rattle/lexer.hpp>
#include <stdexcept>
#include <unordered_map>

namespace rattle::lexer {
  Token consume_comment(State &state) {
    while (not state.empty() and state.peek() != '\n') {
      state.advance();
    }
    return state.make_token(Token::Kind::HashTag);
  }

  void consume_space(State &state) {
    while (not state.empty()) {
      switch (state.peek()) {
      default:
        goto outside;
      case ' ':
      case '\t':
        state.advance();
      }
    }
  outside:
    state.consume_lexeme();
  }

  static const std::unordered_map<std::string_view, Token::Kind> keywords = {
#define TK_MACRO(kind, strrep) {strrep, Token::Kind::kind},
#define TK_INCLUDE TK_KEYWORD
#include <rattle/token_macro.hpp>
  };

  Token consume_identifier(State &state) {
    while (not state.empty() and isalnum(state.peek())) {
      state.advance();
    }
    try {
      auto tk = keywords.at(state.lexeme());
      return state.make_token(tk);
    } catch (std::out_of_range &) {
      return state.make_token(Token::Kind::Identifier);
    }
  }

  bool isalnum(char c) { return std::isalnum(c) or c == '_'; }

  bool isspace(char c) {
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
      return true;
    default:
      return false;
    }
  }

  bool numend(char c) {
    switch (c) {
#define TK_INCLUDE TK_FIRSTALL
#define TK_MACRO(_, ch) case ch:
#include <rattle/token_macro.hpp>
    case '\\':
      return true;
    default:
      return isspace(c);
    }
  }

  bool isbin(char c) {
    switch (c) {
    case '0':
    case '1':
      return true;
    default:
      return false;
    }
  }

  bool isoct(char c) {
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      return true;
    default:
      return false;
    }
  }

  bool isdec(char c) {
    switch (c) {
    case '8':
    case '9':
      return true;
    default:
      return isoct(c);
    }
  }

  bool ishex(char c) {
    switch (c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      return true;
    default:
      return isdec(c);
    }
  }

  const char *to_string(Token::Kind kind) {
    switch (kind) {
#define TK_MACRO(type, _)                                                      \
  case Token::Kind::type:                                                      \
    return #type;
#include <rattle/token_macro.hpp>
    default:
      return "<UnknownTokenKind>";
    }
  }

  const char *to_string(error_t error) {
    switch (error) {
#define ERROR_MACRO(err)                                                       \
  case error_t::err:                                                           \
    return #err;
#define ERROR_INCLUDE LEXER_ERROR
#include <rattle/error_macro.hpp>
    default:
      return "<UnknownLexerErrorT>";
    }
  }

  std::string_view token_content(std::string const &content, std::size_t start,
                                 std::size_t end) {
    return {&(*(content.begin() + start)), end - start};
  }
} // namespace rattle::lexer

