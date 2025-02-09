#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <lexer/lexer.hpp>
#include <optional>
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
  std::deque<std::string_view> lines;
  std::vector<rattle::lexer::error_t> errors;
  lexer_manager_t(): lines{{""}}, errors{} {}
  onerror report_error(rattle::lexer::error_t error) override {
    errors.push_back(error);
    return onerror::keep_going;
  }
  void cache_line(std::size_t lineno, std::string_view line) override {
    lines.push_back(line);
  }
};

void lex_file(const char *filepath) {
  std::string const content = read_file(filepath).value_or("");
  lexer_manager_t manager;
  rattle::lexer::lexer_t lexer{content, manager};
  while (not lexer.empty()) {
    auto token = lexer.lex();
    std::cout << token << '\n';
  }
  for (auto &error : manager.errors) {
    std::cerr << error << '\n';
  }
}

int main(int argc, char **argv) {
  std::span files(argv + 1, static_cast<size_t>(argc - 1));
  std::ranges::for_each(files, lex_file);
}

