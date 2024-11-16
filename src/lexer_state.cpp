#include <rattle/lexer.hpp>
#include <string_view>

namespace rattle::lexer {
  bool State::empty() const { return _iter == content.end(); }
  char State::peek(std::ptrdiff_t n) const { return *(_iter + n); }
  void State::replace_char(char new_char) { *(_iter - 1) = new_char; }
  bool State::safe(std::size_t n) const { return max_safe() > n; }
  std::size_t State::max_safe() const { return content.end() - _iter; }
  std::string_view State::lexeme() const { return {_lex_start, _iter}; }
  bool State::match_next(char expected) {
    return (advance(), safe() and match(expected));
  }
  bool State::match(char expected) {
    return *_iter == expected ? (advance(), true) : false;
  }
  void State::erase_char() {
    advance();
    content.erase(_iter - 1);
  }
  void State::consume_lexeme() {
    _lex_loc = _cur_loc;
    _lex_start = _iter;
  }
  Location State::lexeme_location() const {
    return {.line = _lex_loc.line,
            .column = _lex_loc.offset - _lex_loc.column,
            .offset = _lex_loc.offset};
  }
  Location State::current_location() const {
    return {.line = _cur_loc.line,
            .column = _cur_loc.offset - _cur_loc.column,
            .offset = _cur_loc.offset};
  }
  token::Token State::make_token(token::Token::Kind kind) {
    token::Token token = {
      .kind = kind, .start = lexeme_location(), .end = current_location()};
    consume_lexeme();
    return token;
  }
  char State::advance() {
    char advanced = *_iter++;
    _cur_loc.offset++;
    if (advanced == '\n') {
      _cur_loc.column = _cur_loc.offset;
      _cur_loc.line++;
    }
    return advanced;
  }
  void State::report(error_t error) {
    errors.emplace(error, lexeme_location(), current_location());
  }
  void State::report(error_t error, Location start) {
    errors.emplace(error, start, current_location());
  }
  void State::report(error_t error, Location start, Location end) {
    errors.emplace(error, start, end);
  }
  token::Token State::make_token(error_t error) {
    return (report(error), make_token(token::Token::Kind::Error));
  }
} // namespace rattle::lexer

