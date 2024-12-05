#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <rattle/analyzer.hpp>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>
#include <span>

namespace fs = std::filesystem;

template <bool procloc = false> struct PrintableToken {
  rattle::lexer::Token const &token;
  std::string const &content;
};

std::ostream &operator<<(std::ostream &out,
                         rattle::lexer::Location const &loc) {
  return out << "Loc(l=" << loc.line << ", c=" << loc.column
             << ", o=" << loc.offset << ')';
}

std::ostream &operator<<(std::ostream &out,
                         rattle::lexer::__detail::proc_loc const &p) {
  return out << '(' << p.start << ", " << p.end << ", " << (p.end - p.start)
             << ')';
}

template <bool procloc>
std::ostream &operator<<(std::ostream &out, PrintableToken<procloc> const &p) {
  std::string_view content;
  switch (p.token.kind) {
  case rattle::lexer::Token::Kind::Eos:
    content = "(Eos)";
    break;
  case rattle::lexer::Token::Kind::Eot:
    content = "(Eot)";
    break;
  default:
    content = rattle::lexer::token_content(
      p.content, procloc ? p.token.proc.start : p.token.start.offset,
      procloc ? p.token.proc.end : p.token.end.offset);
  }
  return out << "Token(" << rattle::lexer::to_string(p.token.kind)
             << ", \x1b[32m" << content << "\x1b[0m, start=" << p.token.start
             << ", end=" << p.token.end << ", .proc=" << p.token.proc << ')';
}

void _lex_file(std::string content, std::string const &file) {
  rattle::Lexer lexer(content);
#ifdef SHOW_PROC_TK
  std::string const &pstr = lexer.get_content();
#endif
  for (;;) {
    rattle::lexer::Token token = lexer.scan();
    std::cout << PrintableToken(token, content) << '\n';
#ifdef SHOW_PROC_TK
    std::cout << PrintableToken<true>(token, pstr) << '\n';
#endif
    if (token.kind == rattle::lexer::Token::Kind::Eot) {
      break;
    }
  }
  for (auto &error : lexer.errors) {
    std::cerr << file << ": Error(\x1b[31m"
              << rattle::lexer::to_string(error.type) << "\x1b[0m, \x1b[91;1m"
              << rattle::lexer::token_content(content, error.start.offset,
                                              error.end.offset)
              << "\x1b[0m, " << error.start << ", " << error.end << ")\n";
  }
}

void _parse_file(std::string content, std::string const &file) {
  rattle::Parser parser;
  parser.reset(content);
  auto stmts = parser.parse();
  rattle::analyzer::NodeType node_type;
  for (auto &stmt : stmts) {
    /*std::cout << "Type: " << PrintableToken(stmt->token, content) << '\n';*/
    stmt->visit(node_type);
    std::cout << "Type: "
              << rattle::analyzer::to_string(node_type.get_type(*stmt)) << '\n';
  }
  std::cout << "Parser emitted " << parser.errors.size() << " errors\n";
  while (parser.errors.size()) {
    rattle::parser::Error &error = parser.errors.front();
    std::cout << "Error(\x1b[91m" << rattle::parser::to_string(error.type)
              << "\x1b[0m, start=" << error.start << ", end=" << error.end
              << ")\n";
    parser.errors.pop_front();
  }
}

void lex_file(fs::path const &file) {
  if (fs::is_regular_file(file)) {
    std::ifstream reader(file, std::ios::in);
    if (reader.is_open()) {
      std::size_t size = fs::file_size(file);
      std::string buffer(size, 0);
      reader.read(buffer.data(), size);
      /*_lex_file(buffer, file);*/
      /*std::cout << "---------------------------------------------------\n";*/
      std::cout << "FILE: " << file << '\n';
      _parse_file(std::move(buffer), file);
      std::cout << "--------------------------------------------------\n";
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

