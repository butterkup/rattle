#include <deque>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>

namespace rattle::parser {
  lexer::Token State::get(bool ignore_eos, bool ignore_comments) {
    auto const ignore = [ & ](lexer::Token const &t) {
      return (ignore_eos and t.kind == lexer::Token::Kind::Eos) or
             (ignore_comments and t.kind == lexer::Token::Kind::HashTag);
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
  bool State::empty() const { return hit_eot; }
  void State::unget(lexer::Token const &token) { stash.push_back(token); }

  State::State(Lexer &lexer, std::deque<lexer::Token> &stash,
               std::deque<Error> &errors)
    : lexer(lexer), stash(stash), errors(errors), hit_eot(false) {}
  State::State(Lexer &lexer, std::deque<lexer::Token> &stash,
               std::deque<Error> &errors, State const &state)
    : lexer(lexer), stash(stash), errors(errors), hit_eot(state.hit_eot) {}

} // namespace rattle::parser

