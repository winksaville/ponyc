#define DBG_ENABLED true
#include "../../../src/libponyrt/dbg/dbg_ctx_impl_file.h"
#include "../../../src/libponyrt/dbg/dbg_ctx_impl_file_internal.h"

#include <gtest/gtest.h>
#include <platform.h>

class DbgImplFileTest : public testing::Test
{};

#if !defined(PLATFORM_IS_WINDOWS) && !defined(PLATFORM_IS_MACOSX)
TEST_F(DbgImplFileTest, PrintStr)
{
  char buffer[0x100];
  char expected[0x100];
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  dbg_ctx_t* dc = dbg_ctx_impl_file_create(1, memfile);
  dbg_ctx_impl_file_t* dcif = (dbg_ctx_impl_file_t*)dc;
  dbg_ctx_common_t* dcc = &dcif->common;
  ASSERT_TRUE(dc != NULL);

  // Test id is expected value
  EXPECT_EQ(dcc->id, DBG_CTX_ID_FILE);
  EXPECT_EQ(dcc->id & 0xFFFF, DBG_CTX_ID_COMMON & 0xFFFF);

  snprintf(expected, sizeof(expected), "TestBody:%-4d hi\n", __LINE__ + 1);
  dbg_printf(DBG_LOC(dc), "hi\n");

  fclose(memfile);
  EXPECT_STREQ(expected, buffer);

  dbg_ctx_destroy(dc);
}
#endif

#if !defined(PLATFORM_IS_WINDOWS) && !defined(PLATFORM_IS_MACOSX)
TEST_F(DbgImplFileTest, PrintFormat)
{
  char buffer[0x100];
  char expected[0x100];
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  dbg_ctx_t* dc = dbg_ctx_impl_file_create(1, memfile);

  snprintf(expected, sizeof(expected),
      "TestBody:%-4d Yo Dude\n", __LINE__ + 1);
  dbg_printf(DBG_LOC(dc), "Yo %s\n", "Dude");

  fclose(memfile);
  EXPECT_STREQ(expected, buffer);

  dbg_ctx_destroy(dc);
}
#endif
