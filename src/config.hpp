#ifndef _rat_config
#define _rat_config

#include <string>
#include <string_view>
#include <vector>
#include <queue>

namespace rat {
  struct Source {
    std::string content;
    std::vector<std::string_view> lines;
    Source(): content(), lines() { }
    Source(std::string const& content)
      : content(content), lines() { }
  };

  enum class Phase {
    lexing, parsing, analysis
  };

  struct Location {
    std::size_t line, column, offset, length;
  };


  struct SyntaxError {
    Phase phase;
    std::string_view message;
    Location location;
  };

  struct Config {
    Source source;
    std::queue<SyntaxError> errors;
    Config(): source(), errors() { }
  };
}

#endif //_rat_config
