#include "dummy.h"
#include "../ast/id.h"
#include "../pass/expr.h"
#include "../../libponyrt/mem/pool.h"
#include "ponyassert.h"
#include <string.h>

// Change to true to see the debug output
#define LOCAL_DBG_PASS true
#include "dbg_pass.h"

static void test1_dbg()
{
  DBGE();
  DB("No function name ");
  DB("v=%d\n", 123);
  DBGX();
}

static void test2_dbg()
{
  DBGES("Entering");
  DBG("Between DBGE and DBGX\n");
  DBGXS("Exiting");
}

static void test3_dbg()
{
  DBGES("Entering");
  DBG("Between %s and %s\n", "DBGE", "DBGX");
  DBGXRS(3, "Exiting");
}

static void test4_dbg(ast_t* ast)
{
  DAPE(ast);
  DAP(ast);
  DAPX(ast);
}

static void test5_dbg(ast_t* ast)
{
  DAPE(ast);
  DAP(ast);
  DAPXR(ast, 5);
}

static void test6_dbg(ast_t* ast)
{
  DAPES(ast, "Entering");
  DAPS(ast, "a string");
  DAPXS(ast, "Exiting");
}

static void test7_dbg(ast_t* ast)
{
  DAPES(ast, "Entering");
  DAPS(ast, "a string");
  DAPXRS(ast, 7, "Exiting");
}

static void test8_dbg(ast_t* ast)
{
  DAPE(ast);
  DBG("Current ast and 2 parents");
  DAPP(ast, 3);
  DBG("Print current ast's siblings");
  DAS(ast);
  DAPX(ast);
}

ast_result_t pass_pre_dummy(ast_t** astp, pass_opt_t* options)
{
  UNUSED(options);
  DBGE();
  ast_t* ast = *astp;

  switch(ast_id(ast))
  {
    default: {DAPXVS(ast, ast_id(ast), "default");}
  }

  DBGXR(AST_OK);
  return AST_OK;
}

ast_result_t pass_dummy(ast_t** astp, pass_opt_t* options)
{
  UNUSED(options);
  DBGE();
  ast_t* ast = *astp;

  // Test the debug code once
  static bool once = false;
  if (!once)
  {
    once = true;
    test1_dbg();
    test2_dbg();
    test3_dbg();
    test4_dbg(ast);
    test5_dbg(ast);
    test6_dbg(ast);
    test7_dbg(ast);
    test8_dbg(ast);
  }

  bool r = true;
  switch(ast_id(ast))
  {
    // Add TK_xx cases here such as
    case TK_REFERENCE: r = true; break;

    default: {DAPXVS(ast, ast_id(ast), "default");}
  }

  ast_result_t result = pass_check_result(r, options);
  DBGXR(result);
  return result;
}
