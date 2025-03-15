#include "ensure_def.h"

// Pretty anticlimactic, one would think there should be thousands of token
// variants but they are nested and thats the trick, further distinction
// is held by a bit storage (int) that will be interpreted as per the token
// kind field, neat, right?

// Also a marker but added here since it is a major token type in the lexer,
// an empty lexer yields this kind indefinitely.
rattle_pp_token_macro(Eot, "") // Marks end of token stream/file

// All tokens can be placed in one of these and the flags used to distinguish them further
#include "literal.h"
rattle_pp_token_macro(Marker, "") // Hold other symbols that mark something (start/end/existence/event); }, #, {, ...
rattle_pp_token_macro(Operator, "") // Assignment variants; +, -, *, /, ., ...
rattle_pp_token_macro(Assignment, "") // Assignment variants; =, -=, *=, ...
