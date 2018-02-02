#include "dummy.h"
#include "../ast/id.h"
#include "../pass/expr.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "../dbg/dbg_util.h"

// Uncomment to use stub
#define USE_STUB true

// Uncomment to enable
//#define DBG_ENABLED true
#include "../dbg/dbg.h"

// Uncomment to enable AST debug
//#define DBG_AST_ENABLED true
#include "../dbg/dbg_ast.h"

static void test0_dbg(void)
{
  dbg_ctx_t* dctx = dc_init(stdout, 1024);

  pony_assert(dctx != NULL);

  //D(DBG_FILE, "_DC_BIT_IDX(dummy, 0)=%d\n", _DC_BI(dummy, 0));
  //D(DBG_FILE, "_DC_BIT_IDX(dummy, 1)=%d\n", _DC_BI(dummy, 1));
  uint32_t bit_idx_0 = dc_bni(dummy, 0);
  uint32_t bit_idx_1 = dc_bni(dummy, 1);
  pony_assert(bit_idx_0 == 0x20);
  pony_assert(bit_idx_1 == 0x21);

  // Save original value
  bool dummy_0 = dc_gb(dctx, dc_bni(dummy, 0));
  bool dummy_1 = dc_gb(dctx, dc_bni(dummy, 1));
  //D(DBG_FILE, "dummy_0=%0x\n", dummy_0);
  //D(DBG_FILE, "dummy_1=%0x\n", dummy_1);
  pony_assert(dc_gb(dctx, dc_bni(dummy, 0)) == false);
  pony_assert(dc_gb(dctx, dc_bni(dummy, 1)) == false);

  // Change both bits to 1 and verify the proper bits got changed
  dc_sb(dctx, dc_bni(dummy, 0), 1);
  dc_sb(dctx, dc_bni(dummy, 1), 1);
  //D(DBG_FILE, "dc_gb(dctx, dc_bni(dummy, 0))=%0x\n",
  //    dc_gb(dctx, dc_bni(dummy, 0)));
  //D(DBG_FILE, "dc_gb(dctx, dc_bni(dummy, 1))=%0x\n",
  //    dc_gb(dctx, dc_bni(dummy, 1)));
  pony_assert(dc_gb(dctx, dc_bni(dummy, 0)) == true);
  pony_assert(dc_gb(dctx, dc_bni(dummy, 1)) == true);
  for(uint32_t i=0; i < sizeof(dctx->bits)/sizeof(dctx->bits[0]); i++)
  {
    if(i == 1)
      pony_assert(dctx->bits[i] == 0b11);
    else
      pony_assert(dctx->bits[i] == 0);
  }

  // Change 0 to 0 and we should only get one message
  dc_sb(dctx, dc_bni(dummy, 1), 0);
  DX(dctx, dc_gb(dctx, dc_bni(dummy, 0)), "dummy 0: Hello, %s\n", "World");
  DX(dctx, dc_gb(dctx, dc_bni(dummy, 1)), "dummy 1: Hello, %s\n", "World");

  // Restore original value
  dc_sb(dctx, dc_bni(dummy, 0), dummy_0);
  dc_sb(dctx, dc_bni(dummy, 1), dummy_1);
  pony_assert(dc_gb(dctx, dc_bni(dummy, 0)) == dummy_0);
  pony_assert(dc_gb(dctx, dc_bni(dummy, 1)) == dummy_1);

  dc_destroy(dctx);
}

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

// Change to false for stub
#if !USE_STUB

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
    test0_dbg();
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

#else

ast_result_t pass_pre_dummy(ast_t** astp, pass_opt_t* options)
{
  MAYBE_UNUSED(options);
  MAYBE_UNUSED(astp);
  return AST_OK;
}

ast_result_t pass_dummy(ast_t** astp, pass_opt_t* options) {
  MAYBE_UNUSED(astp);
  MAYBE_UNUSED(options);
  ast_t* ast = *astp;
  static bool once = false;
  if (!once)
  {
    once = true;
    test0_dbg();
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
  return AST_OK;
}

#endif
