#include <deque>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>

namespace rattle::parser {
  lexer::Token State::get(bool ignore_eos, bool ignore_comments) {
    if (stash.empty()) {
      while (true) {
        auto token = lexer.scan();
        hit_eot = token.kind == lexer::Token::Kind::Eot;
        if (ignore_comments and token.kind == lexer::Token::Kind::HashTag) {
          continue;
        }
        if (ignore_eos and token.kind == lexer::Token::Kind::Eos) {
          continue;
        }
        return token;
      }
    }
    auto token = stash.back();
    stash.pop_back();
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
  void State::unget(lexer::Token const &token) { stash.push_back(token); }

  State::State(Lexer &lexer, std::deque<lexer::Token> &stash,
               std::deque<Error> &errors)
    : lexer(lexer), stash(stash), errors(errors), hit_eot(false) {}
  State::State(Lexer &lexer, std::deque<lexer::Token> &stash,
               std::deque<Error> &errors, State const &state)
    : lexer(lexer), stash(stash), errors(errors), hit_eot(state.hit_eot) {}

} // namespace rattle::parser

