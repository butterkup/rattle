#include "../ensure_def.h"

// Prevent misuse if mistakenly included for all keywords in which
// this expands to an invalid token as opposed to the keywords
// that yield the keyword's string payload.
rattle_pp_token_macro(Variable, *) // non keyword name in the program
#include "../keywords/all.h" // Keywords are simply special identifiers

