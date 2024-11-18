#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <rattle/lexer.hpp>
#include <span>

namespace fs = std::filesystem;

struct PrintableToken {
  rattle::lexer::Token const &token;
  std::string const &content;
};

std::ostream &operator<<(std::ostream &out,
                         rattle::lexer::Location const &loc) {
  return out << "Loc(l=" << loc.line << ", c=" << loc.column
             << ", o=" << loc.offset << ')';
}

std::ostream &operator<<(std::ostream &out, PrintableToken const &p) {
  std::string_view content;
  switch (p.token.kind) {
  case rattle::lexer::Token::Kind::Eos:
    content = "(Eos)";
    break;
  case rattle::lexer::Token::Kind::Eot:
    content = "(Eot)";
    break;
  default:
    content = rattle::lexer::token_content(p.content, p.token.start.offset,
                                           p.token.end.offset);
  }
  return out << "Token(" << rattle::lexer::to_string(p.token.kind)
             << ", \x1b[91;1m" << content << "\x1b[0m, start=" << p.token.start
             << ", end=" << p.token.end << ')';
}

void _lex_file(std::string &&content, std::string const &file) {
  rattle::Lexer lexer(content);
  for (;;) {
    rattle::lexer::Token token = lexer.scan();
    std::cout << PrintableToken(token, content) << '\n';
    if (token.kind == rattle::lexer::Token::Kind::Eot) {
      break;
    }
  }
  while (lexer.errors.size()) {
    rattle::lexer::Error &error = lexer.errors.front();
    std::cerr << file << ": Error(" << rattle::lexer::to_string(error.type)
              << ", " << error.start << ", " << error.end << ")\n";
    lexer.errors.pop_front();
  }
}

void lex_file(fs::path const &file) {
  if (fs::is_regular_file(file)) {
    std::ifstream reader(file, std::ios::in);
    if (reader.is_open()) {
      std::size_t size = fs::file_size(file);
      std::string buffer(size, 0);
      reader.read(buffer.data(), size);
      _lex_file(std::move(buffer), file);
    } else {
      std::cerr << "Failed opening file: \x1b[91m" << file << "\x1b[0m\n";
    }
  } else {
    std::cerr << "Not a regular file: \x1b[91m" << file << "\x1b[0m\n";
  }
}

int main(int argc, char **argv) {
  std::span files(argv + 1, static_cast<size_t>(argc - 1));
  std::ranges::for_each(files, lex_file);
}

