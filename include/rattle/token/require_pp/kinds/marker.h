#include "../ensure_def.h"

rattle_pp_token_macro(OpenBrace, "{") // Start of block statement
rattle_pp_token_macro(CloseBrace, "}") // End of block statement
rattle_pp_token_macro(OpenBracket, "[") // Start of list or subscript expression
rattle_pp_token_macro(CloseBracket, "]") // End of list or subscript expression
rattle_pp_token_macro(OpenParen, "(") // Start of tuple or group expression
rattle_pp_token_macro(CloseParen, ")") // End of tuple of group expression
rattle_pp_token_macro(Pound, "#") // Marks the start of a comment

rattle_pp_token_macro(Error, "Error Occurred") // If the lexer encounters and error like invalid character
rattle_pp_token_macro(Whitespace, " \t\r\f") // Black spaces; all characters must belong to a token
rattle_pp_token_macro(Semicolon, ";") // Statement terminator: semicolon
rattle_pp_token_macro(Newline, "\n") // Statement terminator: newline
rattle_pp_token_macro(Escape, "\\n") // Top level escape like escaped newline
