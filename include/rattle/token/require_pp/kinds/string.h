#include "../ensure_def.h"

// The numbers in the second argument hold the flag that says
// the feature/aspect of the string is on
rattle_pp_token_macro(Normal, 0) // Well formed singleline escaped string
// If off, the string is well formed
rattle_pp_token_macro(Error, 1) // Some error occurred while lexing
// If off, the string escape sequences are escaped
rattle_pp_token_macro(Raw, 2) // Unescaped string; ignore escapes.
// If off, the string is Single line which is implied by `Normal`
rattle_pp_token_macro(Multiline, 4) // Multiline string
