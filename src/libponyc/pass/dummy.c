#include "dummy.h"
#include "../ast/id.h"
#include "../pass/expr.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "../dbg/dbg_util.h"

// Uncomment to enable
#define DBG_ENABLED true
#include "../dbg/dbg.h"

// Uncomment to enable AST debug
#define DBG_AST_ENABLED true
#include "../dbg/dbg_ast.h"

// Initialize debug context
static dbg_ctx_t* dc = NULL;
INITIALIZER(initialize)
{
  dc = dbg_ctx_create_with_dst_file(stderr, 32);
  fprintf(stderr, "initialize:# %s\n", __FILE__);
}

// Finalize debug context
FINALIZER(finalize)
{
  dbg_ctx_destroy(dc);
  dc = NULL;
  fprintf(stderr, "finalize:# %s\n", __FILE__);
}

static inline void maybe_enable_debug(ast_t* ast, pass_opt_t* opt)
{
  // If initialize didn't run, for instance in Windows do it now.
  if(dc == NULL)
  {
    initialize();
  }

  UNUSED(opt);
  switch(ast_id(ast))
  {
    case TK_CLASS:
    {
      const char* name = ast_name(ast_child(ast));
      if(strcmp(name, "C") == 0)
      {
        //dbg_sb(dc, 0, 1);
        //dbg_sb(dc, 1, 1);
        dbg_sb(dc, 2, 1);
        DBG_PFNU(dc, "TK_CLASS name=\"%s\"\n", name);
        DBG_PSNU(dc, "TK_CLASS name=\"C\"\n");
      }
      break;
    }
    default: {}
  }
}

static inline void maybe_disable_debug(ast_t* ast, pass_opt_t* opt)
{
  UNUSED(opt);
  switch(ast_id(ast))
  {
    case TK_CLASS:
    {
      const char* name = ast_name(ast_child(ast));
      if(strcmp(name, "C") == 0)
      {
        dbg_sb(dc, 0, 0);
        dbg_sb(dc, 1, 0);
        dbg_sb(dc, 2, 0);
        DBG_PFNU(dc, "TK_CLASS name=\"%s\"\n", name);
      }
      break;
    }
    default: {}
  }
}

ast_result_t pass_pre_dummy(ast_t** astp, pass_opt_t* options)
{
  maybe_enable_debug(*astp, options);

  ast_t* ast = *astp;
  DBG_E(dc, 0);
  DBG_AST(dc, 1, ast);

  switch(ast_id(ast))
  {
    case TK_CLASS: // Windows insists you should have cases
    default: { DBG_ASTF(dc, 2, ast, "id=%d default ", ast_id(ast)); }
  }

  DBG_PFX(dc, 0, "r=%d\n", AST_OK);
  return AST_OK;
}

ast_result_t pass_dummy(ast_t** astp, pass_opt_t* options)
{
  maybe_disable_debug(*astp, options);

  DBG_E(dc, 0);
  ast_t* ast = *astp;

  bool r = true;
  switch(ast_id(ast))
  {
    // Add TK_xx cases here such as
    case TK_REFERENCE: r = true; break;

    default: { DBG_ASTF(dc, 2, ast, "id=%d default ", ast_id(ast)); }
  }

  ast_result_t result = pass_check_result(r, options);
  DBG_PFX(dc, 0, "r=%d\n", result);
  return result;
}
