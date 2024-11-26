#pragma once
// clang-format off
#define LEXER_ERROR   0x0001
#define PARSER_ERROR  0x0002

#define TK_KEYUNARY   0x0001
#define TK_KEYBINARY  0x0002
#define TK_KEYOTHER   0x0004
#define TK_SYMBOL     0x0008
#define TK_SPECIAL    0x0010
#define TK_FIRST      0x0020
#define TK_ASSIGN     0x0040
#define TK_PRIMARY    0x0080
#define TK_SINGLE     0x0100
#define TK_OPOTHER    0x0200
#define TK_OPCMP      0x0400
#define TK_PRI_NUMBER 0x0800
#define TK_KEYLITERAL 0x1000

#define TK_FIRSTALL   (TK_OPALL | TK_ASSIGN | TK_FIRST | TK_SYMBOL | TK_SINGLE)
#define TK_ALL        ~(TK_PRI_NUMBER | TK_SINGLE)
#define TK_KEYEXPR    (TK_KEYUNARY | TK_KEYBINARY)
#define TK_KEYWORD    (TK_KEYEXPR | TK_KEYOTHER)
#define TK_OPALL      (TK_OPOTHER | TK_OPCMP)

