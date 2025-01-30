#include <rattle/lexer.hpp>
#include <string_view>

namespace rattle::lexer {
  bool State::empty() const { return iter == content.end(); }
  char State::peek(std::ptrdiff_t n) const { return *(iter + n); }
  char State::advance() { return this->advance_loc(*iter++); }
  bool State::safe(std::size_t n) const { return max_safe() > n; }
  std::size_t State::max_safe() const { return content.end() - iter; }

  void State::consume_lexeme() {
    lexloc = curloc;
    lexstart = iter;
  }

  bool State::match_next(char expected) {
    return (advance(), safe() and match(expected));
  }

  bool State::match(char expected) {
    return *iter == expected ? (advance(), true) : false;
  }

  std::string_view State::lexeme() const {
    return {lexloc.offset, curloc.offset};
  }

  Location State::lexeme_location() const { return lexloc; }

  Location State::current_location() const { return curloc; }

  Token State::make_token(Token::Kind kind) {
    Token token = {
      .kind = kind, .start = lexeme_location(), .end = current_location()};
    consume_lexeme();
    return token;
  }

  char State::advance_loc(char consumed) {
    curloc.offset++;
    if (consumed == '\n') {
      curloc.column = 0;
      curloc.line++;
    }
    return consumed;
  }

  void State::report(error_t error) {
    errors.emplace_back(error, lexeme_location(), current_location());
  }

  void State::report(error_t error, Location start) {
    errors.emplace_back(error, start, current_location());
  }

  void State::report(error_t error, Location start, Location end) {
    errors.emplace_back(error, start, end);
  }

  Token State::make_token(error_t error) {
    return (report(error), make_token(Token::Kind::Error));
  }
} // namespace rattle::lexer

