#include "transform_ref_to_this.h"
#include "ponyassert.h"

// Uncomment to enable
//#define DBG_AST_ENABLED true
#include "../dbg/dbg_ast.h"

static bool transform_ref_to_this(pass_opt_t* opt, ast_t** astp)
{
  UNUSED(opt);
  ast_t* ast = *astp;
  DASTE(ast);

  // Assume everything we reference is in scope
  // as pass_detect_undefined_refs was successful
  sym_status_t status;
  ast_t* def = ast_get(ast, ast_name(ast_child(ast)), &status);
  pony_assert(def != NULL);

  switch(ast_id(def))
  {
    case TK_FVAR:
    case TK_FLET:
    case TK_EMBED:
    case TK_NEW:
    case TK_BE:
    case TK_FUN:
    {
      // Transform to "this.f".
      ast_t* dot = ast_from(ast, TK_DOT);
      ast_add(dot, ast_child(ast));

      ast_t* self = ast_from(ast, TK_THIS);
      ast_add(dot, self);

      ast_replace(astp, dot);
      ast = *astp;
      break;
    }

    default: {}
  }

  DASTXR(true, ast);
  return true;
}

ast_result_t pass_transform_ref_to_this(ast_t** astp, pass_opt_t* options)
{
  DPLE();
  ast_t* ast = *astp;

  bool r = true;
  switch(ast_id(ast))
  {
    case TK_REFERENCE: r = transform_ref_to_this(options, astp); break;

    default: { DASTF(ast, "id=%d default ", ast_id(ast)); }
  }

  ast_result_t result = pass_check_result(r, options);
  DPLX("r=%d", result);
  return result;
}
