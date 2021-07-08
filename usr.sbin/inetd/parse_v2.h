#include "inetd.h"

typedef enum parse_v2_result {V2_SUCCESS, V2_SKIP, V2_ERROR} parse_v2_result;

/* 
 * Parse a key-values service definition, starting at the token after
 * on/off (i.e. parse a series of key-values pairs terminated by a semicolon).
 * Fills the provided servtab structure. Does not call freeconfig on error.
 */
parse_v2_result	parse_syntax_v2(struct servtab *, char **);