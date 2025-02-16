#include "ensure_def.h"

#ifndef rattle_markers_h
# define rattle_markers_h

rattle_pp_token_macro(Error, "Error Occurred")    // If the lexer encounters and error
rattle_pp_token_macro(Eot, "End Of Token stream") // Marks end of token stream/file
rattle_pp_token_macro(Whitespace, " \t\r\f")      // Black spaces.
rattle_pp_token_macro(Semicolon, ";")             // Statement terminators:
rattle_pp_token_macro(Newline, "\n")              // Newline and Semicolon
rattle_pp_token_macro(Escape, "")                 // Top level escape

#endif
