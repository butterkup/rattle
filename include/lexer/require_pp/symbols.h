#include "ensure_defined.h"

// Symbols

TOKEN_MACRO(OpenBrace, "{")
TOKEN_MACRO(CloseBrace, "}")
TOKEN_MACRO(OpenBracket, "[")
TOKEN_MACRO(CloseBracket, "]")
TOKEN_MACRO(OpenParen, "(")
TOKEN_MACRO(CloseParen, ")")
TOKEN_MACRO(Comma, ",")
TOKEN_MACRO(Hashtag, "#")

// Special symbols (Markers)

TOKEN_MACRO(Error, "") // If the lexer encounters and error
TOKEN_MACRO(Eot, "") // Marks end of token stream/file
TOKEN_MACRO(Semicolon, ";") // Statement terminators
TOKEN_MACRO(Newline, "\n")
