#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <queue>

namespace rat::lexer {
  namespace utils {
    bool isidchar(char c) {
      return std::isalnum(c) or c == '_';
    }
    bool isoperator(char c) {
      switch ( c ) {
      case '+':
      case '%':
      case '>':
      case '<':
      case '=':
      case '!':
      case '*':
      case '/':
      case '-': return true;
      default: return false;
      }
    }
    bool isspace(char c) {
      switch ( c ) {
      case ' ':
      case '\t':
      case '\v':
      case '\r':
      case '\n': return true;
      default: return false;
      }
    }
    bool isbinary(char c) {
      switch ( c ) {
      case '0':
      case '1': return true;
      default: return false;
      }
    }
    bool isoctal(char c) {
      switch ( c ) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7': return true;
      default: return false;
      }
    }
    bool isdecimal(char c) {
      if ( isoctal(c) ) return true;
      switch ( c ) {
      case '8':
      case '9': return true;
      default: return false;
      }
    }

    bool ishexadecimal(char c) {
      if ( isdecimal(c) ) return true;
      switch ( c ) {
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F': return true;
      default: return false;
      }
    }
  }

  struct StringView {
    using Type = std::string::iterator;
    Type begin, end;
    StringView()
      : begin(), end() { }
    StringView(Type begin, Type end)
      : begin(begin), end(end) { }
  };

  struct Location {
    std::size_t line;
    std::string::difference_type
      column, offset;
  };

  struct Source {
    std::string content;
    std::vector<StringView> lines;

    Source(): content{ }, lines{ } { }
    Source(std::string const& content)
      : content{ content }, lines{ } { }
    Source(std::string&& content)
      : content{ std::move(content) }, lines{ } { }
    Source(Source const&) = default;
    Source(Source&&) = default;
  };

  struct SyntaxError {
    std::string_view msg, fix;
    Location start, end;
  };

  struct Config {
    Source source;
    std::queue<SyntaxError> errors;
  };
}
