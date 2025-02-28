#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <rattle/lexer/lexer.hpp>
#include <rattle/utility.hpp>
#include <span>
#include <vector>

std::optional<std::string> read_file(const char *filepath) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (not file) {
    return std::nullopt;
  }
  std::string content(file.tellg(), '\0');
  file.seekg(0);
  file.read(content.data(), content.size());
  return content;
}

struct LexerReactor: rattle::lexer::IReactor {
  std::vector<rattle::lexer::error::Error> errors;
  LexerReactor(): errors{} {};

  rattle::lexer::OnError report(rattle::lexer::error::Error error) noexcept {
    errors.push_back(error);
    return rattle::lexer::OnError::Resume;
  }

  void show_errors() const {
    for (auto &error : errors) {
      std::cerr << error << '\n';
    }
  }

  void trace(rattle::token::Token &token) noexcept {
    std::cout << token << '\n';
  }
};

void lex_file(const char *filepath) {
  std::string const rawpath = to_string(rattle::utility::escape{filepath});
  if (auto content = read_file(filepath)) {
    std::cout << "------------------[ " << rawpath << " ]------------------\n";
    LexerReactor reactor;
    rattle::lexer::Lexer lexer{*content, reactor};
    while (not lexer.empty()) {
      lexer.lex();
    }
    std::cerr << "-------------[ ERRORS: " << rawpath << " ]-------------\n";
    reactor.show_errors();
  } else {
    std::cerr << rawpath << ": Error reading file." << '\n';
  }
}

int main(int argc, char **argv) {
  std::span filepaths(argv + 1, static_cast<size_t>(argc - 1));
  std::ranges::for_each(filepaths, lex_file);
}

