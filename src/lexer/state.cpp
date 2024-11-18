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

  void State::advance_erase() {
    advance_loc(*iter);
    iter = content.erase(iter);
  }

  void State::advance_replace(char new_char) {
    advance_loc(*iter);
    *iter++ = new_char;
  }

  std::string_view State::lexeme() const {
    auto size = curloc.offset - lexloc.offset;
    return {&(*(iter - size)), size};
  }

  Location State::lexeme_location() const {
    return {.line = lexloc.line,
            .column = lexloc.offset - lexloc.column,
            .offset = lexloc.offset};
  }

  Location State::current_location() const {
    return {.line = curloc.line,
            .column = curloc.offset - curloc.column,
            .offset = curloc.offset};
  }

  __detail::proc_loc State::proc_loc() const {
    auto const begin = content.begin();
    return {.start = static_cast<std::size_t>(lexstart - begin),
            .end = static_cast<std::size_t>(iter - begin)};
  }

  Token State::make_token(Token::Kind kind) {
    Token token = {.kind = kind,
                   .start = lexeme_location(),
                   .end = current_location(),
                   .proc = proc_loc()};
    consume_lexeme();
    return token;
  }

  char State::advance_loc(char consumed) {
    curloc.offset++;
    if (consumed == '\n') {
      curloc.column = curloc.offset;
      curloc.line++;
    }
    return consumed;
  }

  void State::reset() {
    lexstart = iter = content.begin();
    lexloc = curloc = {1, 0, 0};
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

