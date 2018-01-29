#ifndef PASS_DETECT_UNDEFINED_REFS_H
#define PASS_DETECT_UNDEFINED_REFS_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

ast_result_t pass_detect_undefined_refs(ast_t** astp, pass_opt_t* options);

PONY_EXTERN_C_END

#endif

