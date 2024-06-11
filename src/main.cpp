#include <iostream>

#include "lexer/scanner.hpp"
#include <filesystem>
#include <string>
#include <fstream>
#include <span>

auto main(int argc, char** argv) -> int {
  for ( std::string_view const filepath : std::span{ argv + 1, static_cast<std::size_t>(argc - 1) } ) {
    std::filesystem::path path{ filepath };
    std::size_t size = std::filesystem::file_size(path);
    std::string buffer(size, 0);
    std::ifstream file{ path };
    std::cout << "Lexing file: '" << filepath << "'\n";
    file.read(buffer.data(), size);

    rat::lexer::Config cfg{ std::move(buffer), { } };
    rat::lexer::State state{ cfg };

    rat::lexer::Token token;
    for ( size = 1;; size++ ) {
      token = rat::lexer::scan(state);
      std::cout << size << ".  " << token << '\n';
      if ( token.kind == rat::lexer::Token::Kind::Eot ) break;
    }
    if ( state.cfg.errors.size() > 0 ) {
      std::cout << "\n Errors encountered in: '" << filepath << "'\n";
      size = 0;
      while ( (size++, state.cfg.errors.size() > 0) ) {
        std::cout << ' ' << size << ".  " << state.cfg.errors.front().msg << '\n';
        state.cfg.errors.pop();
      }
    }
    std::cout << '\n';
  }
}