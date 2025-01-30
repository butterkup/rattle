#ifndef RATTLE_SOURCE_ONLY
# include <algorithm>
# include <cassert>
# include <filesystem>
# include <fstream>
# include <iostream>
# include <span>
#endif

#include <rattle/analyzer.hpp>
#include <rattle/lexer.hpp>
#include <rattle/parser.hpp>

namespace fs = std::filesystem;

std::ostream &operator<<(std::ostream &out,
                         rattle::lexer::Location const &loc) {
  return out << "Loc(l=" << loc.line << ", c=" << loc.column
             << ", o=" << loc.offset << ')';
}

std::ostream &operator<<(std::ostream &out, rattle::lexer::Token const &p) {
  std::string_view content;
  switch (p.kind) {
  case rattle::lexer::Token::Kind::Newline:
    content = ";";
    break;
  case rattle::lexer::Token::Kind::Eot:
    content = "";
    break;
  default:
    content = p.payload();
  }
  return out << "Token(" << rattle::lexer::to_string(p.kind)
             << ", \x1b[32m" << content << "\x1b[0m, start=" << p.start
             << ", end=" << p.end << ')';
}

void _lex_file(std::string content, std::string const &file) {
  rattle::Lexer lexer(content);
#ifdef SHOW_PROC_TK
  std::string const &pstr = lexer.get_content();
#endif
  for (;;) {
    rattle::lexer::Token token = lexer.scan();
    std::cout << token << '\n';
    if (token.kind == rattle::lexer::Token::Kind::Eot) {
      break;
    }
  }
  for (auto &error : lexer.errors) {
    std::cerr << file << ": Error(\x1b[31m" << to_string(error.type)
              << "\x1b[0m, \x1b[91;1m"
              << error.payload()
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
    if (node_type.get_type(*stmt) == rattle::analyzer::node_t::ExprStatement) {
      auto expr_stmt =
        static_cast<rattle::parser::nodes::ExprStatement *>(stmt.get());
      std::cout << "Type: " << to_string(node_type.get_type(*expr_stmt->expr))
                << '\n';
    } else {
      std::cout << "Type: " << to_string(node_type.get_type(*stmt)) << '\n';
    }
  }
  std::cout << "Parser emitted " << parser.errors.size() << " errors\n";
  for (auto &error : parser.errors) {
    std::cerr << "Error(\x1b[91m" << to_string(error.type)
              << "\x1b[0m, start=" << error.start << ", end=" << error.end
              << ")\n";
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

