
#include <deque>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>

namespace rattle::parser {
  lexer::Token State::get(bool ignore_comments) {
    if (stash.empty()) {
      auto token = lexer.scan();
      hit_eot = token.kind == lexer::Token::Kind::Eot;
      return token;
    }
    auto token = std::move(stash.front());
    stash.pop_front();
    return token;
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
  void State::unget(lexer::Token const &token) { stash.push_front(token); }

  State::State(Lexer &lexer, std::deque<lexer::Token> &stash,
               std::deque<Error> &errors)
    : lexer(lexer), stash(stash), errors(errors), hit_eot(false) {}
  State::State(Lexer &lexer, std::deque<lexer::Token> &stash,
               std::deque<Error> &errors, State const &state)
    : lexer(lexer), stash(stash), errors(errors), hit_eot(state.hit_eot) {}

} // namespace rattle::parser

