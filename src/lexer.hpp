#ifndef _rat_lexer
#define _rat_lexer

#include "config.hpp"
#include <string>
#include <string_view>
#include "tokens.hpp"
#include <stdexcept>
#include <unordered_map>
#include <cctype>

namespace rat {
  const auto Keywords = [] {
    using enum Token::Kind;
    return std::unordered_map<std::string_view, Token::Kind>{
      {"raise", Raise}, { "class", Class }, { "fn", Fn }, { "let", Let }, { "as", As },
      { "import", Import }, { "in", In }, { "not", Not }, { "and", And }, { "or", Or },
      { "try", Try }, { "else", Else }, { "if", If }, { "while", While }, { "for", For },
      { "except", Except }, { "lastly", Lastly }, { "return", Return }, { "with", With },
      { "assert", Assert }, { "from", From }, { "true", True }, { "false", False }
    };
  }();

  namespace _lex_impl {
    bool isidch(char c) { return std::isalnum(c) or c == '_'; }
    bool isspace(char c) {
      return std::isspace(c);
      // switch ( c ) {
      // case ' ':
      // case '\t':
      // case '\v':
      // case '\r':
      // case '\n': return true;
      // default: return false;
      // }
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


  struct Lexer {
    Config& cfg;
    std::string::const_iterator current, end, line_start, start;
    Lexer() = delete;
    Lexer(Config& cfg):
      cfg(cfg),
      current(cfg.source.content.cbegin()),
      end(cfg.source.content.cend()),
      line_start(current),
      start(current) { }

    char advance() {
      char advanced = this->peek();
      this->current++;
      if ( advanced == '\n' ) {
        this->cfg.source.lines.emplace_back(this->line_start, this->current);
        this->line_start = this->current;
      }
      return advanced;
    }

    std::string_view lexeme() {
      return { this->start, this->current };
    }

    void consume() {
      this->start = this->current;
    }

    Token make_token(Token::Kind kind) {
      Token token{ kind, this->lexeme(), this->location() };
      this->consume();
      return token;
    }

    inline char peek() {
      return *this->current;
    }

    bool match(char expected) {
      return not this->empty() and
        this->peek() == expected ?
        (this->advance(), true) : false;
    }

    bool match_next(char expected) {
      return (this->advance(), this->match(expected));
    }

    bool empty() {
      return this->current == this->end;
    }

    Location location() {
      return {
        .line = this->cfg.source.lines.size() + 1,
        .column = this->current - this->line_start,
        .offset = this->current - this->cfg.source.content.cbegin(),
        .length = this->current - this->start
      };
    }

    void report_error(std::string_view message) {
      this->cfg.errors.push({
        .phase = Phase::lexing,
        .message = message,
        .location = this->location()
        });
    }

    void consume_comment() {
      while ( not this->empty() and this->peek() != '\n' )
        this->advance();
    }

    void consume_space() {
      while ( not this->empty() )
        if ( _lex_impl::isspace(this->peek()) )
          this->advance();
        else break;
    }

    Token::Kind consume_identifier() {
      while ( not this->empty() )
        if ( _lex_impl::isidch(this->peek()) )
          this->advance();
        else break;
      try {
        return Keywords.at(this->lexeme());
      } catch ( std::out_of_range ) {
        return Token::Kind::Identifier;
      }
    }

    Token::Kind consume_number() { return Token::Kind::Error; }
    Token::Kind consume_long_string() { return Token::Kind::Error; }
    Token::Kind consume_string() { return Token::Kind::Error; }

    Token scan() {
      using enum Token::Kind;
      while ( (this->consume_space(), not this->empty()) ) {
        switch ( this->peek() ) {
        case '#': return this->make_token((this->consume_comment(), HashTag));
        case '>':
          return this->match_next('>') ?
            this->make_token(this->match_next('=') ? RshiftEqual : Rshift) :
            this->make_token(this->match_next('=') ? GreaterEqual : Greater);
        case '<':
          return this->match_next('<') ?
            this->make_token(this->match_next('=') ? LshiftEqual : Lshift) :
            this->make_token(this->match_next('=') ? LessEqual : Less);
        case '+': return this->make_token(this->match_next('=') ? PlusEqual : Plus);
        case '*': return this->make_token(this->match_next('=') ? StarEqual : Star);
        case '-': return this->make_token(this->match_next('=') ? MinusEqual : Minus);
        case '/': return this->make_token(this->match_next('=') ? SlashEqual : Slash);
        case '=': return this->make_token(this->match_next('=') ? EqualEqual : Equal);
        case '!': return this->make_token(
          this->match_next('=') ? NotEqual : (
            this->report_error("incomplete operator '!=', '!' must be followed by '='."),
            Error
            ));
        case '%': return this->make_token(this->match_next('=') ? PercentEqual : Percent);
        case '.': return this->make_token((this->advance(), Dot));
        case ',': return this->make_token((this->advance(), Comma));
        case '[': return this->make_token((this->advance(), OpenBracket));
        case ']': return this->make_token((this->advance(), CloseBracket));
        case '(': return this->make_token((this->advance(), OpenParen));
        case ')': return this->make_token((this->advance(), CloseParen));
        case '{': return this->make_token((this->advance(), OpenBrace));
        case '}': return this->make_token((this->advance(), CloseBrace));
        case ';': return this->make_token((this->advance(), Semicolon));
        default:
          if ( std::isdigit(this->peek()) )
            return this->make_token(this->consume_number());
          else if ( _lex_impl::isidch(this->peek()) )
            return this->make_token(this->consume_identifier());
        }
      }
      return this->make_token(Eot);
    }
  };
}

#endif //_rat_lexer
