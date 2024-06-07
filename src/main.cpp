#include <iostream>
#include <fstream>
#include <filesystem>
#include <span>
#include "config.hpp"
#include "lexer.hpp"
#include "tokens.hpp"

auto main(int argc, char** argv) -> int {
  for ( std::filesystem::path path : std::span{ argv + 1, argc - 1 } ) {
    std::cout << "Lexing file: " << path << '\n';
    std::ifstream file{ path };
    rat::Config cfg;
    std::string buffer;
    while ( not file.eof() ) {
      std::getline(file, buffer);
      cfg.source.content += buffer;
    }
    rat::Lexer lex{ cfg };
    rat::Token tk;
    for ( std::size_t count = 1; count; count++ ) {
      tk = lex.scan();
      std::cout << count << ": " << tk << '\n';
      if ( tk.kind == rat::Token::Kind::Eot ) break;
    }
    std::cout << "Errors encountered\n";
    rat::SyntaxError error;
    while(cfg.errors.size()) {
      error = cfg.errors.front();
      std::cout << error.message << " found in " << error.location << '\n';
    }
  }
}
