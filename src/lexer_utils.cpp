#include "lexer.hpp"
#include <rattle/lexer.hpp>
#include <rattle/token_category.hpp>
#include <stdexcept>

namespace rattle::lexer {
  token::Token consume_comment(Lexer &lexer) {
    while (not lexer.state.empty()) {
      lexer.state.advance();
    }
    return lexer.state.make_token(token::Token::Kind::HashTag);
  }

  void consume_space(Lexer &lexer) {
    while (not lexer.state.empty()) {
      switch (lexer.state.peek()) {
      case ' ':
      case '\t':
        lexer.state.advance();
      default:
        goto outside;
      }
    }
  outside:
    lexer.state.consume_lexeme();
  }

  token::Token consume_identifier(Lexer &lexer) {
    while (not lexer.state.empty() and isalnum(lexer.state.peek())) {
      lexer.state.advance();
    }
    try {
      auto tk = token::keywords.at(lexer.state.lexeme());
      return lexer.state.make_token(tk);
    } catch (std::out_of_range) {
      return lexer.state.make_token(token::Token::Kind::Identifier);
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
#define TK_INCLUDE TK_SINGLEALL
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
} // namespace rattle::lexer

