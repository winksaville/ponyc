#include <gtest/gtest.h>
#include <platform.h>

#include <ast/astbuild.h>
#include <ast/ast.h>
#include <pass/scope.h>
#include <ast/stringtab.h>
#include <pass/pass.h>
#include <pkg/package.h>
#include <pkg/use.h>

#include "util.h"

#define DBG_ENABLED true
#include "../../src/libponyrt/dbg/dbg.h"
#include "../../src/libponyrt/dbg/dbg_ctx_impl_file.h"

#define DBG_AST_ENABLED true
#define DBG_AST_PRINT_WIDTH 40
#include "../../src/libponyc/ast/dbg_ast.h"


#define TEST_COMPILE(src) DO(test_compile(src, "scope"))
#define TEST_ERROR(src) DO(test_error(src, "scope"))


class DbgAstTest : public PassTest
{
#if !defined(PLATFORM_IS_WINDOWS) && !defined(PLATFORM_IS_MACOSX)
  protected:
    virtual void SetUp() {
      memfile = fmemopen(buffer, sizeof(buffer), "w+");
      ASSERT_TRUE(memfile != NULL);

      // Create dc set bit 0
      dc = dbg_ctx_impl_file_create(1, memfile);
      dbg_sb(dc, 0, 1);
      // Create a "program" but I'm not sure what second parameter
      // to BUILD should be, for now just create an TK_ID?
      ast_t* node = ast_blank(TK_ID);
      ast_set_name(node, "test");
      BUILD(prog, node,
        NODE(TK_PACKAGE, AST_SCOPE
          NODE(TK_MODULE, AST_SCOPE
            NODE(TK_ACTOR, AST_SCOPE
              ID("main")
              NONE
              NODE(TK_TAG)
              NONE
              NODE(TK_MEMBERS,
                NODE(TK_NEW, AST_SCOPE
                  NODE(TK_TAG)
                  ID("create")  // name
                  NONE          // typeparams
                  NONE          // params
                  NONE          // return type
                  NONE          // error
                  NODE(TK_SEQ, NODE(TK_TRUE))
                  NONE
                  NONE
                )
              )
            )
          )
        )
      );
      program = prog;
    }

    virtual void TearDown() {
      dbg_ctx_destroy(dc);
      fclose(memfile);
    }

    char buffer[8192];
    FILE* memfile;
    dbg_ctx_t* dc;
    ast_t* program;
#endif
};

#if !defined(PLATFORM_IS_WINDOWS) && !defined(PLATFORM_IS_MACOSX)
TEST_F(DbgAstTest, DbgAst)
{
  char expected[0x200];

  snprintf(expected, sizeof(expected),
      "TestBody:%-4d program (package:scope\n"
      "  (module:scope\n"
      "    (actor:scope\n"
      "      (id main)\n"
      "      x\n"
      "      tag\n"
      "      x\n"
      "      (members\n"
      "        (new:scope\n"
      "          tag\n"
      "          (id create)\n"
      "          x\n"
      "          x\n"
      "          x\n"
      "          x\n"
      "          (seq true)\n"
      "          x\n"
      "          x\n"
      "        )\n"
      "      )\n"
      "    )\n"
      "  )\n"
      ")\n", __LINE__ + 1);
  if(DBG(dc, 0)) DBG_AST(dc, program);
  fflush(memfile);

  // Test if successful
  //printf("ast_text len=%zu buffer len=%zu\n",
  //  strlen(ast_text), strlen(buffer));
  if(strcmp(expected, buffer) != 0)
  {
    ASSERT_TRUE(strcmp(expected, buffer) == 0) <<
      "expected:" << std::endl <<
      "\"" << expected << "\"" << std::endl <<
      "got:" << std::endl <<
      "\"" << buffer << "\"";
  }
}

TEST_F(DbgAstTest, DbgAstf)
{
  char expected[0x200];

  ast_t* module = ast_child(program);
  ast_t* actor = ast_child(module);
  ast_t* id = ast_child(actor);

  snprintf(expected, sizeof(expected),
    "TestBody:%-4d id ast_child(actor)=(id main)\n", __LINE__ + 1);
  if(DBG(dc, 0)) DBG_ASTF(dc, id, "ast_child(actor)=");
  fflush(memfile);
  //printf("expected len=%zu buffer len=%zu\n",
  //  strlen(expected), strlen(buffer));
  if(strcmp(expected, buffer) != 0)
  {
    ASSERT_TRUE(strcmp(expected, buffer) == 0) <<
      "expected:" << std::endl <<
      "\"" << expected << "\"" << std::endl <<
      "got:" << std::endl <<
      "\"" << buffer << "\"";
  }
}

TEST_F(DbgAstTest, DbgAstp)
{
  char expected[0x200];

  ast_t* module = ast_child(program);
  ast_t* actor = ast_child(module);
  ast_t* id = ast_child(actor);

  snprintf(expected, sizeof(expected),
    "TestBody:%-4d id[0]: (id main)\n"
    "TestBody:%-4d id[-1]: (actor:scope\n"
    "  (id main)\n"
    "  x\n"
    "  tag\n"
    "  x\n"
    "  (members\n"
    "    (new:scope\n"
    "      tag\n"
    "      (id create)\n"
    "      x\n"
    "      x\n"
    "      x\n"
    "      x\n"
    "      (seq true)\n"
    "      x\n"
    "      x\n"
    "    )\n"
    "  )\n"
    ")\n", __LINE__ + 1, __LINE__ + 1);
  DBG_ASTP(dc, id, 2);
  fflush(memfile);
  //printf("expected len=%zu buffer len=%zu\n",
  //  strlen(expected), strlen(buffer));
  if(strcmp(expected, buffer) != 0)
  {
    ASSERT_TRUE(strcmp(expected, buffer) == 0) <<
      "expected:" << std::endl <<
      "\"" << expected << "\"" << std::endl <<
      "got:" << std::endl <<
      "\"" << buffer << "\"";
  }
}

TEST_F(DbgAstTest, DbgAsts)
{
  char expected[0x200];

  ast_t* module = ast_child(program);
  ast_t* actor = ast_child(module);
  ast_t* id = ast_child(actor);

  snprintf(expected, sizeof(expected),
    "TestBody:%-4d id[0]: (id main)\n"
    "TestBody:%-4d id[1]: x\n"
    "TestBody:%-4d id[2]: tag\n"
    "TestBody:%-4d id[3]: x\n"
    "TestBody:%-4d id[4]: (members\n"
    "  (new:scope\n"
    "    tag\n"
    "    (id create)\n"
    "    x\n"
    "    x\n"
    "    x\n"
    "    x\n"
    "    (seq true)\n"
    "    x\n"
    "    x\n"
    "  )\n"
    ")\n", __LINE__ + 1, __LINE__ + 1, __LINE__ + 1, __LINE__ +1, __LINE__ + 1);
  DBG_ASTS(dc, id);
  fflush(memfile);
  //printf("expected len=%zu buffer len=%zu\n",
  //  strlen(expected), strlen(buffer));
  if(strcmp(expected, buffer) != 0)
  {
    ASSERT_TRUE(strcmp(expected, buffer) == 0) <<
      "expected:" << std::endl <<
      "\"" << expected << "\"" << std::endl <<
      "got:" << std::endl <<
      "\"" << buffer << "\"";
  }
}
#endif
