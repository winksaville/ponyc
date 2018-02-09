#include <gtest/gtest.h>
#include <platform.h>

#include "../../src/libponyc/dbg/dbg_util.h"

#define DBG_ENABLED true
#include "../../src/libponyc/dbg/dbg.h"

/**
 * Example of bits defined by a base_name and offset
 * xxxx_bit_idx where base name is xxxx and xxxx_bit_idx
 * is the offset for the first bit for base name in the
 * bits array. xxxx_bit_cnt is the number of bit reserved
 * for xxxx.
 */
enum {
  DBG_BITS_FIRST(first, 30),
  DBG_BITS_NEXT(second, 3, first),
  DBG_BITS_NEXT(another, 2, second),
  DBG_BITS_SIZE(bits_size, another)
};

class DbgTest : public testing::Test
{};

TEST_F(DbgTest, DbgDoEmpty)
{
  // Should compile
  _DBG_DO();
}

TEST_F(DbgTest, DbgPsnuEasy)
{
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(stdout, 3);
  ASSERT_TRUE(dc != NULL);

  DBG_PSNU(dc, "hi\n");

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfnuEasy)
{
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(stdout, 3);
  ASSERT_TRUE(dc != NULL);

  DBG_PFNU(dc, "Yo %s\n", "Dude");

  dbg_ctx_destroy(dc);
}

#ifndef _MSC_VER
TEST_F(DbgTest, DbgDoSingleStatement)
{
  char buffer[8192];
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  _DBG_DO(fprintf(memfile, "Hi"));
  fclose(memfile);
  EXPECT_STREQ(buffer, "Hi");
}

TEST_F(DbgTest, DbgDoMultiStatement)
{
  char buffer[8192];
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  _DBG_DO(int i = 0; fprintf(memfile, "i=%d", i));
  //fprintf(stderr, "i=%d", i); // This fails as i isn't in scope
  fclose(memfile);
  EXPECT_STREQ(buffer, "i=0");
}

TEST_F(DbgTest, DbgBitsArrayIdx)
{
  EXPECT_EQ(_DBG_BITS_ARRAY_IDX(0), 0);
  EXPECT_EQ(_DBG_BITS_ARRAY_IDX(31), 0);
  EXPECT_EQ(_DBG_BITS_ARRAY_IDX(32), 1);
  EXPECT_EQ(_DBG_BITS_ARRAY_IDX(63), 1);
  EXPECT_EQ(_DBG_BITS_ARRAY_IDX(64), 2);
  EXPECT_EQ(_DBG_BITS_ARRAY_IDX(95), 2);
}

TEST_F(DbgTest, DbgBitMask)
{
  EXPECT_EQ(_DBG_BIT_MASK(0),  0x00000001);
  EXPECT_EQ(_DBG_BIT_MASK(1),  0x00000002);
  EXPECT_EQ(_DBG_BIT_MASK(31), 0x80000000);
  EXPECT_EQ(_DBG_BIT_MASK(32), 0x00000001);
  EXPECT_EQ(_DBG_BIT_MASK(63), 0x80000000);
  EXPECT_EQ(_DBG_BIT_MASK(64), 0x00000001);
  EXPECT_EQ(_DBG_BIT_MASK(95), 0x80000000);
}

TEST_F(DbgTest, DbgCreateDestroy)
{
  // Verify data structure
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file((FILE*)0x1000, 1);
  printf("DbgCreateDestroy: dc=%p\n", dc);
  printf("DbgCreateDestroy: &dc->dst_file=%p\n", &dc->dst_file);
  printf("DbgCreateDestroy: &dc->dst_buf=%p\n", &dc->dst_buf);
  printf("DbgCreateDestroy: &dc->dst_buf_size=%p\n", &dc->dst_buf_size);
  printf("DbgCreateDestroy: &dc->dst_buf_begi=%p\n", &dc->dst_buf_begi);
  printf("DbgCreateDestroy: &dc->dst_buf_endi=%p\n", &dc->dst_buf_endi);
  printf("DbgCreateDestroy: &dc->dst_buf_cnt=%p\n", &dc->dst_buf_cnt);
  printf("DbgCreateDestroy: &dc->bits=%p\n", &dc->bits);
  printf("DbgCreateDestroy: &dc->tmp_buf_size=%p\n", &dc->tmp_buf_size);
  printf("DbgCreateDestroy: &dc->tmp_buf=%p\n", &dc->tmp_buf);
  printf("DbgCreateDestroy: &dc->max_size=%p\n", &dc->max_size);
  printf("DbgCreateDestroy: dc->bits=%p\n", dc->bits);
  printf("DbgCreateDestroy: dc->tmp_buf_size=%zu\n", dc->tmp_buf_size);
  ASSERT_TRUE(dc->bits != NULL);
  EXPECT_EQ(dc->bits[0], 0);
  EXPECT_TRUE(dc->dst_file == (FILE*)0x1000);
  EXPECT_TRUE(dc->dst_buf == NULL);
  EXPECT_EQ(dc->dst_buf_size, 0);
  dbg_ctx_destroy(dc);

  dc = dbg_ctx_create_with_dst_file(stdout, 1);
  ASSERT_TRUE(dc->bits != NULL);
  EXPECT_EQ(dc->bits[0], 0);
  EXPECT_TRUE(dc->dst_file == stdout);
  EXPECT_TRUE(dc->dst_buf == NULL);
  EXPECT_EQ(dc->dst_buf_size, 0);
  dbg_ctx_destroy(dc);

  dc = dbg_ctx_create_with_dst_buf(0x1000, 0x100, 1);
  ASSERT_TRUE(dc->bits != NULL);
  EXPECT_EQ(dc->bits[0], 0);
  EXPECT_TRUE(dc->dst_file == NULL);
  EXPECT_TRUE(dc->dst_buf != NULL);
  EXPECT_EQ(dc->dst_buf_size, 0x1000); // - 1);
  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgCtxBitsInitToZero)
{
  const uint32_t num_bits = 65;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(stderr, num_bits);

  // Verify all bits are zero
  for(uint32_t bi = 0; bi < num_bits; bi++)
  {
    EXPECT_FALSE(dbg_gb(dc, bi));
  }

  // Verify bits array is zero
  for(uint32_t i = 0; i < (num_bits + 31) / 32; i++)
  {
    EXPECT_EQ(dc->bits[i], 0);
  }

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, WalkingOneBit)
{
  const uint32_t num_bits = 29;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(stderr, num_bits);

  // Walking one bit
  for(uint32_t bi = 0; bi < num_bits; bi++)
  {
    bool v = dbg_gb(dc, bi);
    EXPECT_FALSE(v);
    dbg_sb(dc, bi, 1);
    v = dbg_gb(dc, bi);
    EXPECT_TRUE(v);
    
    // Verify only one bit is set
    for(uint32_t cbi = 0; cbi < num_bits; cbi++)
    {
      bool v = dbg_gb(dc, cbi);
      if (cbi == bi)
        EXPECT_TRUE(v);
      else
        EXPECT_FALSE(v);
    }

    // Verify only one bit is set using bits array
    uint32_t bits_array_idx = bi / 32;
    uint32_t bits_mask = 1 << (bi % 32);
    for(uint32_t i = 0; i < (num_bits + 31) / 32; i++)
    {
      if(i == bits_array_idx)
        EXPECT_EQ(dc->bits[i], bits_mask);
      else
        EXPECT_EQ(dc->bits[i], 0);
    }
    dbg_sb(dc, bi, 0);
  }

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, WalkingTwoBits)
{
  const uint32_t num_bits = 147;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(stderr, num_bits);

  // Number bits must be >= 2
  EXPECT_TRUE(num_bits >= 2);

  // Walking two bits
  dbg_sb(dc, 0, 1);
  for(uint32_t bi = 1; bi < num_bits - 1; bi++)
  {
    // Get first of the pair it should be 1
    bool v = dbg_gb(dc, bi - 1);
    EXPECT_TRUE(v);

    // Get second bit it should be 0
    v = dbg_gb(dc, bi);
    EXPECT_FALSE(v);
    
    // Set second bit
    dbg_sb(dc, bi, 1);

    // Verify two bits are set
    for(uint32_t cbi = 0; cbi < num_bits; cbi++)
    {
      bool v = dbg_gb(dc, cbi);
      if (cbi == (bi - 1))
        EXPECT_TRUE(v);
      else if (cbi == bi)
        EXPECT_TRUE(v);
      else
        EXPECT_FALSE(v);
    }
    
    // Clear first bit
    dbg_sb(dc, bi - 1, 0);
  }

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWrite)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 0x100, 1);

  // Write "a"
  DBG_PFU(dc, "%s", "a");

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteWrite)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 0x100, 1);

  // Write "a", "b"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");

  // Read and verify "b" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("b", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteWriteWrite)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 0x100, 1);

  // Write "a", "b", "c"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");
  DBG_PFU(dc, "%s", "c");

  // Read and verify "b" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("c", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWrite2)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 0x100, 1);

  // Write "ab"
  DBG_PFU(dc, "%s", "ab");

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWrite3)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 0x100, 1);

  // Write "abc"
  DBG_PFU(dc, "%s", "abc");

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteEqTmpBufSize)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 5, 1);

  // Write "abcde"
  const char* str = "abcde";
  ASSERT_EQ(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteGtTmpBufSize)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 5, 1);

  // Write "abcdef"
  const char* str = "abcdef";
  ASSERT_GT(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteReadRead)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "a"
  DBG_PFU(dc, "%s", "a");

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteReadWriteReadWriteRead)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "a"
  DBG_PFU(dc, "%s", "a");

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Write "b"
  DBG_PFU(dc, "%s", "b");

  // Read and verify "b" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("b", buf);

  // Write "c"
  DBG_PFU(dc, "%s", "c");

  // Read and verify "c" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("c", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufFillEmpty)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "a" then "b"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");

  // Read verify "a"
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("a", buf);

  // Read verify "b"
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("b", buf);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufOverFillBy1Empty)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "a", "b", "c"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");
  DBG_PFU(dc, "%s", "c");

  // Read verify "b"
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("b", buf);

  // Read verify "c"
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("c", buf);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwByteBufOverFillBy2ReadTillEmpty)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "a", "b", "c", "d"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");
  DBG_PFU(dc, "%s", "c");
  DBG_PFU(dc, "%s", "d");

  // Read verify "c"
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("c", buf);

  // Read verify "d"
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 1);
  ASSERT_STREQ("d", buf);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufFillSingleOpReadTillEmpty)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "ab"
  DBG_PFU(dc, "%s", "ab");

  // Read verify "ab"
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  ASSERT_EQ(cnt, 2);
  ASSERT_STREQ("ab", buf);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufOverFillBy1SingleOpReadTillEmpty)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "ab"
  DBG_PFU(dc, "%s", "abc");

  // Read verify "ab"
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  ASSERT_EQ(cnt, 2);
  ASSERT_STREQ("ab", buf);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  ASSERT_EQ(cnt, 0);
  ASSERT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWrite2Read2)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "ab"
  const char* str = "ab";
  DBG_PFU(dc, "%s", str);

  // Read and verify "ab" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("ab", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteEqTmpBufSize)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 5, 1);

  // Write "abcde"
  const char* str = "abcde";
  ASSERT_EQ(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read and verify "ab" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("ab", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteGtTmpBufSize)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 5, 1);

  // Write "abcdef"
  const char* str = "abcdef";
  ASSERT_GT(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read and verify "ab" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("ab", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteWriteReadRead)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "a", "b"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");

  // Read and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Write "c"
  DBG_PFU(dc, "%s", "c");

  // Read and verify "b" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("b", buf);

  // Read and verify "c" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("c", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteReadWrite2Read2)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 0x100, 1);

  // Write "a"
  DBG_PFU(dc, "%s", "a");

  // Read and verify "a"
  cnt = dbg_read(dc, buf, sizeof(buf), 1);
  EXPECT_EQ(cnt, 1);
  EXPECT_STREQ("a", buf);

  // Write "bc"
  DBG_PFU(dc, "%s", "bc");

  // Read and verify "bc"
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 2);
  EXPECT_STREQ("bc", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuThreeByteBufWrite3Read2WriteRead2)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(3, 0x100, 1);

  // Write "abc"
  DBG_PFU(dc, "%s", "abc");

  // Read and verify "ab" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 2);
  //EXPECT_STREQ("ab", buf);
  EXPECT_STREQ("ab", buf);

  // Write "d"
  DBG_PFU(dc, "%s", "d");

  // Read verify "cd" was written
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 2);
  //EXPECT_STREQ("cd", buf);
  EXPECT_STREQ("cd", buf);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf), 2);
  EXPECT_EQ(cnt, 0);
  EXPECT_STREQ("", buf);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgBnoi)
{
  EXPECT_EQ(dbg_bnoi(first,0), 0);
  EXPECT_EQ(dbg_bnoi(first,1), 1);
  EXPECT_EQ(dbg_bnoi(second,0), 30);
  EXPECT_EQ(dbg_bnoi(second,1), 31);
  EXPECT_EQ(dbg_bnoi(second,2), 32);
  EXPECT_EQ(dbg_bnoi(another,0), 33);
  EXPECT_EQ(dbg_bnoi(another,1), 34);
  EXPECT_EQ(bits_size, 30 + 3 + 2);
}

//TEST_F(DbgTest, DbgReadWriteBitsOfSecond)
//{
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(stderr, bits_size);
//
//  // Get bit index of second[0] and second[1] they should be adjacent
//  uint32_t bi0 = dbg_bnoi(second, 0);
//  uint32_t bi1 = dbg_bnoi(second, 1);
//  EXPECT_EQ(bi0 + 1, bi1);
//
//  // Initially they should be zero
//  bool o0 = dbg_gb(dc, bi0);
//  bool o1 = dbg_gb(dc, bi1);
//  EXPECT_FALSE(o0);
//  EXPECT_FALSE(o1);
//
//  // Write new values and verify they changed
//  dbg_sb(dc, bi0, !o0);
//  dbg_sb(dc, bi1, !o1);
//  EXPECT_EQ(dbg_gb(dc, bi0), !o0);
//  EXPECT_EQ(dbg_gb(dc, bi1), !o1);
//
//  // Restore original values
//  dbg_sb(dc, bi0, o0);
//  dbg_sb(dc, bi1, o1);
//  EXPECT_EQ(dbg_gb(dc, bi0), o0);
//  EXPECT_EQ(dbg_gb(dc, bi1), o1);
//
//  dbg_ctx_destroy(dc);
//}
#endif
