#include "mark_dont_care_refs.h"
#include "../ast/id.h"

// Uncomment to enable
//#define DBG_AST_ENABLED true
#include "../dbg/dbg_ast.h"

static bool mark_dont_care_refs(pass_opt_t* opt, ast_t* ast)
{
  UNUSED(opt);
  DASTE(ast);

  const char* name = ast_name(ast_child(ast));
  if(is_name_dontcare(name))
  {
    ast_setid(ast, TK_DONTCAREREF);
  }

  DASTXR(true, ast);
  return true;
}

ast_result_t pass_mark_dont_care_refs(ast_t** astp, pass_opt_t* options)
{
  UNUSED(options);
  DPLE();
  ast_t* ast = *astp;

  bool r = true;
  switch(ast_id(ast))
  {
    case TK_REFERENCE: r = mark_dont_care_refs(options, ast); break;

    default: { DASTF(ast, "id=%d default ", ast_id(ast)); }
  }

  ast_result_t result = pass_check_result(r, options);
  DPLX("r=%d", result);
  return result;
}
