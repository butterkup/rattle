#include "ensure_def.h"

#ifndef rattle_markers_h
# define rattle_markers_h

TOKEN_MACRO(Error, "Error Occurred")    // If the lexer encounters and error
TOKEN_MACRO(Eot, "End Of Token stream") // Marks end of token stream/file
TOKEN_MACRO(Whitespace, " \t\r\f")      // Black spaces.
TOKEN_MACRO(Semicolon, ";")             // Statement terminators:
TOKEN_MACRO(Newline, "\n")              // Newline and Semicolon

#endif
