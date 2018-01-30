#include "detect_undefined_refs.h"
#include "ponyassert.h"

// Uncomment to enable
//#define DBG_AST_ENABLED true
#include "../dbg/dbg_ast.h"

static bool detect_undefined_refs(pass_opt_t* opt, ast_t* ast)
{
  bool result;
  DASTE(ast);

  const char* name = ast_name(ast_child(ast));

  // Everything we reference is defined in the current scope
  sym_status_t status;
  ast_t* def = ast_get(ast, ast_name(ast_child(ast)), &status);
  if(def != NULL)
  {
    // Save the found definition in the AST, so we
    // don't need to look it up again.
    ast_setdata(ast, (void*)def);
    result = true;
  } else {
    // Failing as nothing was found, but also try to suggest an alternate name.
    pass_suggest_alt_name(opt, ast, name);
    result = false;
  }

  DASTXR(result, ast);
  return result;
}

ast_result_t pass_detect_undefined_refs(ast_t** astp, pass_opt_t* options)
{
  DPLE();
  ast_t* ast = *astp;

  bool r = true;
  switch(ast_id(ast))
  {
    case TK_REFERENCE:
    {
      r = detect_undefined_refs(options, ast);
      break;
    }

    default: { DASTF(ast, "id=%d default ", ast_id(ast)); }
  }

  ast_result_t result = pass_check_result(r, options);
  DPLX("r=%d", result);
  return result;
}
