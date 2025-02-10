#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <rattle/lexer/lexer.hpp>
#include <span>
#include <string_view>
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

struct lexer_manager_t: rattle::lexer::manager_t {
  std::vector<rattle::lexer::error_t> errors;
  lexer_manager_t(): errors{} {}
  onerror report(rattle::lexer::error_t error) override {
    errors.push_back(error);
    return onerror::keep_going;
  }
  void cache_line(std::size_t, std::string_view) override {}
};

void lex_file(const char *filepath) {
  if (auto content = read_file(filepath)) {
    std::cout << "------------------[ " << rattle::lexer::escape_t{filepath}
              << " ]------------------\n";
    lexer_manager_t manager;
    rattle::lexer::lexer_t lexer{*content, manager};
    while (not lexer.empty()) {
      auto token = lexer.lex();
      std::cout << token << '\n';
    }
    for (auto &error : manager.errors) {
      std::cerr << filepath << ": " << error << '\n';
    }
  } else {
    std::cerr << filepath << ": Error reading file." << '\n';
  }
}

int main(int argc, char **argv) {
  std::span filepaths(argv + 1, static_cast<size_t>(argc - 1));
  std::ranges::for_each(filepaths, lex_file);
}

