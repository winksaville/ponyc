#define DBG_ENABLED true
#include "../../../src/libponyrt/dbg/dbg_ctx_impl_mem.h"

#include "../../../src/libponyrt/dbg/dbg_ctx_impl_mem_internal.h"

#include <gtest/gtest.h>
#include <platform.h>

class DbgImplMemTest : public testing::Test
{};

TEST_F(DbgImplMemTest, CreateDestroy)
{
  // Create
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 0x1000, 0x100);
  dbg_ctx_impl_mem_t* dcim = (dbg_ctx_impl_mem_t*)dc;
  dbg_ctx_common_t* dcc = &dcim->common;

  // Test id is are expected values
  EXPECT_EQ(dcc->id, DBG_CTX_ID_MEM);
  EXPECT_EQ(dcc->id & 0xFFFF, DBG_CTX_ID_COMMON & 0xFFFF);

  // Verify dst_buf and tmp_buf are writeable
  ASSERT_TRUE(dcc->bits != NULL);
  EXPECT_EQ(dcc->bits[0], 0);
  EXPECT_TRUE(dcim->dst_buf != NULL);
  EXPECT_EQ(dcim->dst_buf_size, 0x1000);
  for(size_t i = 0; i < dcim->dst_buf_size; i++)
    EXPECT_EQ(dcim->dst_buf[i], 0);
  memset(dcim->dst_buf, 0xAA, dcim->dst_buf_size);
  for(size_t i = 0; i < dcim->dst_buf_size; i++)
    EXPECT_EQ(dcim->dst_buf[i], 0xAA);
  memset(dcim->dst_buf, 0x55, dcim->dst_buf_size);
  for(size_t i = 0; i < dcim->dst_buf_size; i++)
    EXPECT_EQ(dcim->dst_buf[i], 0x55);
  memset(dcim->dst_buf, 0x0, dcim->dst_buf_size);
  for(size_t i = 0; i < dcim->dst_buf_size; i++)
    EXPECT_EQ(dcim->dst_buf[i], 0);

  // Verify tmp_buf is an extra byte in length
  EXPECT_TRUE(dcim->tmp_buf != NULL);
  EXPECT_EQ(dcim->tmp_buf_size, 0x100);
  for(size_t i = 0; i < dcim->tmp_buf_size + 1; i++)
    EXPECT_EQ(dcim->tmp_buf[i], 0);
  memset(dcim->tmp_buf, 0xAA, dcim->tmp_buf_size + 1);
  for(size_t i = 0; i < dcim->tmp_buf_size + 1; i++)
    EXPECT_EQ(dcim->tmp_buf[i], 0xAA);
  memset(dcim->tmp_buf, 0x55, dcim->tmp_buf_size + 1);
  for(size_t i = 0; i < dcim->tmp_buf_size + 1; i++)
    EXPECT_EQ(dcim->tmp_buf[i], 0x55);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, OneByteBufWrite)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 1, 0x100);

  // Write "a"
  dbg_printf(dc, "a");

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, OneByteBufWriteWrite)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 1, 0x100);

  // Write "a", "b"
  dbg_printf(dc, "a");
  dbg_printf(dc, "b");

  // Read and verify "b" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("b", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, OneByteBufWriteWriteWrite)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 1, 0x100);

  // Write "a", "b", "c"
  dbg_printf(dc, "a");
  dbg_printf(dc, "b");
  dbg_printf(dc, "c");

  // Read and verify "b" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("c", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, OneByteBufWrite2)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 1, 0x100);

  // Write "ab"
  dbg_printf(dc, "ab");

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, OneByteBufWrite3)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 1, 0x100);

  // Write "abc"
  dbg_printf(dc, "abc");

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, OneByteBufWriteEqTmpBufSize)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 1, 5);

  // Write "abcde"
  const char* str = "abcde";
  ASSERT_EQ(strlen(str), 5);
  dbg_printf(dc, str);

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, OneByteBufWriteGtTmpBufSize)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 1, 5);

  // Write "abcdef"
  const char* str = "abcdef";
  ASSERT_GT(strlen(str), 5);
  dbg_printf(dc, str);

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufWriteReadRead)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "a"
  dbg_printf(dc, "a");

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufWriteReadWriteReadWriteRead)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "a"
  dbg_printf(dc, "a");

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Write "b"
  dbg_printf(dc, "b");

  // Read and verify "b" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("b", read_buf);

  // Write "c"
  dbg_printf(dc, "c");

  // Read and verify "c" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("c", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufFillEmpty)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "a" then "b"
  dbg_printf(dc, "a");
  dbg_printf(dc, "b");

  // Read verify "a"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("a", read_buf);

  // Read verify "b"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("b", read_buf);

  // Read verify ""
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufOverFillBy1Empty)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "a", "b", "c"
  dbg_printf(dc, "a");
  dbg_printf(dc, "b");
  dbg_printf(dc, "c");

  // Read verify "b"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("b", read_buf);

  // Read verify "c"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("c", read_buf);

  // Read verify ""
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwByteBufOverFillBy2ReadTillEmpty)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "a", "b", "c", "d"
  dbg_printf(dc, "a");
  dbg_printf(dc, "b");
  dbg_printf(dc, "c");
  dbg_printf(dc, "d");

  // Read verify "c"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("c", read_buf);

  // Read verify "d"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("d", read_buf);

  // Read verify ""
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufFillSingleOpReadTillEmpty)
{
  char read_buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "ab"
  dbg_printf(dc, "ab");

  // Read verify "ab"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  ASSERT_EQ(cnt, 2);
  ASSERT_STREQ("ab", read_buf);

  // Read verify ""
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufOverFillBy1SingleOpReadTillEmpty)
{
  char read_buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "ab"
  dbg_printf(dc, "abc");

  // Read verify "ab"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  ASSERT_EQ(cnt, 2);
  ASSERT_STREQ("ab", read_buf);

  // Read verify ""
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufWrite2Read2)
{
  char read_buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "ab"
  const char* str = "ab";
  dbg_printf(dc, str);

  // Read and verify "ab" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("ab", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufWriteEqTmpBufSize)
{
  char read_buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 5);

  // Write "abcde"
  const char* str = "abcde";
  ASSERT_EQ(strlen(str), 5);
  dbg_printf(dc, str);

  // Read and verify "ab" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("ab", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufWriteGtTmpBufSize)
{
  char read_buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 5);

  // Write "abcdef"
  const char* str = "abcdef";
  ASSERT_GT(strlen(str), 5);
  dbg_printf(dc, str);

  // Read and verify "ab" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("ab", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufWriteWriteReadRead)
{
  char read_buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "a", "b"
  dbg_printf(dc, "a");
  dbg_printf(dc, "b");

  // Read and verify "a" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Write "c"
  dbg_printf(dc, "c");

  // Read and verify "b" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("b", read_buf);

  // Read and verify "c" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("c", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, TwoByteBufWriteReadWrite2Read2)
{
  char read_buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 2, 0x100);

  // Write "a"
  dbg_printf(dc, "a");

  // Read and verify "a"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", read_buf);

  // Write "bc"
  dbg_printf(dc, "bc");

  // Read and verify "bc"
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("bc", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, ThreeByteBufWrite3Read2WriteRead2)
{
  char read_buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 3, 0x100);

  // Write "abc"
  dbg_printf(dc, "abc");

  // Read and verify "ab" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 2);
  //EXPECT_STREQ("ab", read_buf);
  EXPECT_STREQ("ab", read_buf);

  // Write "d"
  dbg_printf(dc, "d");

  // Read verify "cd" was written
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 2);
  //EXPECT_STREQ("cd", read_buf);
  EXPECT_STREQ("cd", read_buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, DbgLoc)
{
  char read_buf[0x100];
  char expected[0x100];
  const char* class_name;
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 0x100, 0x100);

#ifdef PLATFORM_IS_WINDOWS
  class_name = "DbgImplMemTest_DbgLoc_Test::";
#else
  class_name = "";
#endif

  // Verify no location information printed if DBG_LOC isn't used
  snprintf(expected, sizeof(expected), "hi there");
  dbg_printf(dc, "hi there");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify location is printed if DBG is used
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d \n",
      class_name, __LINE__ + 1);
  dbg_printf(DBG_LOC(dc), "\n");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify second call to dbg_printf doesn't print the location info
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d hi there",
      class_name, __LINE__ + 1);
  dbg_printf(DBG_LOC(dc), "hi"); dbg_printf(dc, " there");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, Dbg)
{
  char read_buf[0x100];
  char expected[0x100];
  const char* class_name;
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 0x100, 0x100);

#ifdef PLATFORM_IS_WINDOWS
  class_name = "DbgImplMemTest_Dbg_Test::";
#else
  class_name = "";
#endif

  // Set bit 0
  dbg_sb(dc, 0, 1);

  // Verify no location information printed if DBG isn't used
  snprintf(expected, sizeof(expected), "hi there");
  dbg_printf(dc, "hi there");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify location is printed if DBG is used
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d \n",
      class_name, __LINE__ + 1);
  DBG(dc, 0); dbg_printf(dc, "\n");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify second call to dbg_printf doesn't print the location info
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d hi there",
      class_name, __LINE__ + 1);
  DBG(dc, 0); dbg_printf(dc, "hi"); dbg_printf(dc, " there");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, Printf)
{
  char read_buf[0x100];
  char expected[0x100];
  const char* class_name;
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 0x100, 0x100);

#ifdef PLATFORM_IS_WINDOWS
  class_name = "DbgImplMemTest_Printf_Test::";
#else
  class_name = "";
#endif

  // Set bit 0
  dbg_sb(dc, 0, 1);

  // Verify no location information printed if DBG isn't used
  snprintf(expected, sizeof(expected), "hi there");
  dbg_printf(dc, "hi there");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify a space follows location information when its printed
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d \n",
      class_name, __LINE__ + 1);
  if(DBG(dc, 0)) dbg_printf(dc, "\n");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify va_args aren't required
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d hi there",
      class_name, __LINE__ + 1);
  if(DBG(dc, 0)) dbg_printf(dc, "hi there");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify va_args work
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d hi there",
      class_name, __LINE__ + 1);
  if(DBG(dc, 0)) dbg_printf(dc, "hi %s", "there");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Verify second call to dbg_printf doesn't print the location info
  snprintf(expected, sizeof(expected), "%sTestBody:%-4d hi there",
      class_name, __LINE__ + 1);
  if(DBG(dc, 0)) { dbg_printf(dc, "hi"); dbg_printf(dc, " there"); }

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgImplMemTest, EnterExit)
{
  char read_buf[0x100];
  char expected[0x100];
  const char* class_name;
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_impl_mem_create(1, 0x100, 0x100);

#ifdef PLATFORM_IS_WINDOWS
  class_name = "DbgImplMemTest_EnterExit_Test::";
#else
  class_name = "";
#endif

  // Set bit 0 and Write
  dbg_sb(dc, 0, 1);

  // Enter Exit plain
  snprintf(expected, sizeof(expected),
      "%sTestBody:%-4d +\n%sTestBody:%-4d -\n",
      class_name, __LINE__ + 1, class_name, __LINE__ + 2);
  if(DBG(dc, 0)) dbg_printf(dc, "+\n");
  if(DBG(dc, 0)) dbg_printf(dc, "-\n");

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  // Enter Exit with args
  snprintf(expected, sizeof(expected),
      "%sTestBody:%-4d + Entered 1\n%sTestBody:%-4d - Exited  2\n",
      class_name, __LINE__ + 1, class_name, __LINE__ + 2);
  if(DBG(dc, 0)) dbg_printf(dc, "+ Entered %d\n", 1);
  if(DBG(dc, 0)) dbg_printf(dc, "- Exited  %d\n", 2);

  // Check it
  cnt = dbg_read(dc, read_buf, sizeof(read_buf), sizeof(read_buf)-1);
  EXPECT_EQ(cnt, strlen(expected));
  EXPECT_STREQ(expected, read_buf);

  dbg_ctx_destroy(dc);
}
