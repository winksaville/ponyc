#include "dummy.h"
#include "../ast/id.h"
#include "../pass/expr.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"
#include <string.h>

// Uncomment to enable
//#define DBG_ENABLED true
#include "../dbg/dbg.h"

// Uncomment to enable
//#define DBG_AST_ENABLED true
#include "../dbg/dbg_ast.h"

static void test1_dbg(void)
{
  DPE(); DP("\n");
  DFP(stdout, "DFP: has no function name v=%d\n", 123);
  DP("DP: has no function name "); DPL("DPL: v=%d", 123);
  DPN("DPN: has function name "); DPL("DPL: v=%d", 456);
  DPLN("DPLN: has function name v=%d %s", 789, "hi");
  DPX(); DP("\n");
}

static void test2_dbg(void)
{
  DPLE();
  DPLX();
}

static void test3_dbg(void)
{
  DPE("DPE: Entering\n");
  DPX("DPX: Exiting\n");
}

static void test4_dbg(void)
{
  DPLE("DPLE: Entering");
  DPLX("DPLX: Exiting");
}

static void test5_dbg(void)
{
  DPLE("DPLE: Entering");
  DPLX("DPLX: %s", "Exiting");
}

static void test6_dbg(void)
{
  DPLE("DPLE: Entering");
  DPLX("DPLX: r=%d", 6);
}

static void test7_dbg(void)
{
  DPLE("DPLE: Entering");
  DPLX("DPLX: r=%d %s", 7, "Exiting");
}

static void test8_dbg(ast_t* ast)
{
  DPLE();
  DAST(ast);
  DPLX();
}

static void test9_dbg(ast_t* ast)
{
  DPLE();
  DASTF(ast, " v=0x%x ", 0x123);
  DPLX();
}

static void test10_dbg(ast_t* ast)
{
  DPLE();
  DPLN("Current ast and 2 parents");
  DASTP(ast, 3);
  DPLN("Print current ast's siblings");
  DASTS(ast);
  DPLX();
}

ast_result_t pass_pre_dummy(ast_t** astp, pass_opt_t* options)
{
  MAYBE_UNUSED(options);
  ast_t* ast = *astp;
  DPLE();
  DAST(ast);

  switch(ast_id(ast))
  {
    default: { DASTF(ast, "id=%d default ", ast_id(ast)); }
  }

  DPLX("r=%d", AST_OK);
  return AST_OK;
}

ast_result_t pass_dummy(ast_t** astp, pass_opt_t* options)
{
  MAYBE_UNUSED(options);
  DPLE();
  ast_t* ast = *astp;

  // Test the debug code once
  static bool once = false;
  if (!once)
  {
    once = true;
    test1_dbg();
    test2_dbg();
    test3_dbg();
    test4_dbg();
    test5_dbg();
    test6_dbg();
    test7_dbg();
    test8_dbg(ast);
    test9_dbg(ast);
    test10_dbg(ast);
  }

  bool r = true;
  switch(ast_id(ast))
  {
    // Add TK_xx cases here such as
    case TK_REFERENCE: r = true; break;

    default: { DASTF(ast, "id=%d default ", ast_id(ast)); }
  }

  ast_result_t result = pass_check_result(r, options);
  DPLX("r=%d", result);
  return result;
}
