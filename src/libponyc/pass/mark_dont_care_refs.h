#ifndef PASS_MARK_DONT_CARE_REFS_H
#define PASS_DUMMY_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

ast_result_t pass_mark_dont_care_refs(ast_t** astp, pass_opt_t* options);

PONY_EXTERN_C_END

#endif
