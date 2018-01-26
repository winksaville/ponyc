#ifndef PASS_TRANSFORM_REF_TO_THIS_H
#define PASS_TRANSFORM_REF_TO_THIS_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

ast_result_t pass_transform_ref_to_this(ast_t** astp, pass_opt_t* options);

PONY_EXTERN_C_END

#endif

