#include <deque>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>

namespace rattle::parser {
  void State::unget(lexer::Token const &token) { stash.push_back(token); }
  bool State::test(char setting) const { return context & setting; }
  char State::setting() const { return context; }
  bool State::empty() const { return hit_eot; }
  bool State::in_paren() const { return scopes.paren; }
  bool State::in_bracket() const { return scopes.bracket; }
  bool State::in_brace() const { return scopes.brace; }
  __detail::Scope State::enter_paren() { return scopes.paren; }
  __detail::Scope State::enter_bracket() { return scopes.bracket; }
  __detail::Scope State::enter_brace() { return scopes.brace; }

  __detail::ContextSetting State::with(char setting) {
    return {*this, setting};
  }

  void State::push(char setting) {
    settings.push_back(context);
    context = setting;
  }

  void State::pop() {
    context = settings.back();
    settings.pop_back();
  }

  lexer::Token State::get() {
    auto const ignore = [ & ](lexer::Token const &t) {
      return (test(IGNORE_EOS) and t.kind == lexer::Token::Kind::Eos) or
             (test(IGNORE_COMMENTS) and t.kind == lexer::Token::Kind::HashTag);
    };
    while (stash.size()) {
      auto token = stash.front();
      stash.pop_front();
      if (ignore(token)) {
        continue;
      }
      return token;
    }
    while (true) {
      auto token = lexer.scan();
      hit_eot = token.kind == lexer::Token::Kind::Eot;
      if (ignore(token)) {
        continue;
      }
      return token;
    }
  }

  const char *to_string(error_t error) {
    switch (error) {
#define ERROR_MACRO(_error)                                                    \
  case error_t::_error:                                                        \
    return #_error;
#define ERROR_INCLUDE PARSER_ERROR
#include <rattle/error_macro.hpp>
    }
  }

  void State::report(error_t error, lexer::Location const &start,
                     lexer::Location const &end) {
    errors.emplace_back(error, start, end);
  }

  void State::report(error_t error, lexer::Token const &start,
                     lexer::Token const &end) {
    errors.emplace_back(error, start.start, end.end);
  }
  void State::report(error_t error, lexer::Token const &token) {
    errors.emplace_back(error, token.start, token.end);
  }
} // namespace rattle::parser

