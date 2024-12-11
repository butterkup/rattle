#pragma once
// clang-format off
#define LEXER_ERROR                        0x00001
#define PARSER_ERROR                       0x00002

#define TK_KEYUNARY                        0x00001
#define TK_KEYBINARY                       0x00002
#define TK_KEYOTHER                        0x00004
#define TK_SYMBOL                          0x00008
#define TK_SPECIAL                         0x00010
#define TK_FIRST                           0x00020
#define TK_ASSIGN                          0x00040
#define TK_PRIMARY                         0x00080
#define TK_SINGLE                          0x00100
#define TK_OPOTHER                         0x00200
#define TK_OPCMP                           0x00400
#define TK_PRINUMBER                       0x00800
#define TK_KEYLITERAL                      0x01000
#define TK_PRIBINARY                       0x02000
#define TK_BASE_NODES                      0x04000
#define TK_PRIUNARY                        0x08000
#define TK_EXTR_NODES                      0x10000
#define TK_CONTAINERS                      0x20000

#define TK_EXPR_NODES (TK_OPALL | TK_KEYUNARY | TK_KEYLITERAL | TK_PRIUNARY | TK_PRIBINARY  | TK_KEYBINARY | TK_PRINUMBER | TK_PRIMARY)
#define TK_ALL        ~(TK_CONTAINERS | TK_PRINUMBER | TK_PRIBINARY | TK_PRIUNARY | TK_SINGLE | TK_BASE_NODES | TK_EXTR_NODES)
#define TK_FIRSTALL   (TK_OPALL | TK_ASSIGN | TK_FIRST | TK_SYMBOL | TK_SINGLE)
#define TK_ALL_NODES  (TK_BASE_NODES | TK_EXPR_NODES | TK_STMT_NODES)
#define TK_STMT_NODES (TK_KEYOTHER | TK_ASSIGN | TK_EXTR_NODES)
#define TK_KEYEXPR    (TK_KEYUNARY | TK_KEYBINARY)
#define TK_KEYWORD    (TK_KEYEXPR | TK_KEYOTHER)
#define TK_OPALL      (TK_OPOTHER | TK_OPCMP)

