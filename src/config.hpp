#ifndef _rat_config
#define _rat_config

#include <string>
#include <string_view>
#include <vector>
#include <queue>

namespace rat::core {
  struct Source {
    std::string source;
    std::vector<std::string_view> lines;
  };

  struct SyntaxError {
    std::string_view message;
    std::size_t
      line,
      column,
      offset,
      span;
  };

  struct Config {
    Source source;
    std::queue<SyntaxError> errors;
  };
}

#endif //_rat_config
