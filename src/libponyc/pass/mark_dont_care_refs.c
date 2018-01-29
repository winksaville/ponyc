#include "mark_dont_care_refs.h"
#include "../ast/id.h"

// Change to true to see the debug output
#define LOCAL_DBG_PASS false
#include "dbg_pass.h"

static bool mark_dont_care_refs(pass_opt_t* opt, ast_t* ast)
{
  UNUSED(opt);
  DAPE(ast);

  const char* name = ast_name(ast_child(ast));
  if(is_name_dontcare(name))
  {
    ast_setid(ast, TK_DONTCAREREF);
  }

  DAPXR(ast, true);
  return true;
}

ast_result_t pass_mark_dont_care_refs(ast_t** astp, pass_opt_t* options)
{
  UNUSED(options);
  DBGE();
  ast_t* ast = *astp;

  bool r = true;
  switch(ast_id(ast))
  {
    case TK_REFERENCE: r = mark_dont_care_refs(options, ast); break;

    default: {DAPXVS(ast, ast_id(ast), "default");}
  }

  ast_result_t result = pass_check_result(r, options);
  DBGXR(result);
  return result;
}
