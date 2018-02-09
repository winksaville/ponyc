#include <gtest/gtest.h>
#include <platform.h>

#include "../../src/libponyc/dbg/dbg_util.h"

#define DBG_ENABLED true
#define DBG_TMP_BUF_SIZE 0x5
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
  EXPECT_EQ(strcmp(buffer, "Hi"), 0);
}

TEST_F(DbgTest, DbgDoMultiStatement)
{
  char buffer[8192];
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  _DBG_DO(int i = 0; fprintf(memfile, "i=%d", i));
  //fprintf(stderr, "i=%d", i); // This fails as i isn't in scope
  fclose(memfile);
  EXPECT_EQ(strcmp(buffer, "i=0"), 0);
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

  dc = dbg_ctx_create_with_dst_buf(0x1000, 1);
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

TEST_F(DbgTest, DbgPfu)
{
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  DBG_PFU(dc, "%s", "a");
  EXPECT_EQ(strcmp("a", dbg_get_buf(dc)), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWrite)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);

  // Write one char, "a"
  DBG_PFU(dc, "%s", "a");

  // Read it back and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("a", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteWrite)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);

  // Write "a", "b"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");

  // Read it back and verify "b" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("b", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteWriteWrite)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);

  // Write "a", "b", "c"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");
  DBG_PFU(dc, "%s", "c");

  // Read it back and verify "b" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("c", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWrite2)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);

  // Write one char, "ab"
  DBG_PFU(dc, "%s", "ab");

  // Read it back and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("a", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWrite3)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);

  // Write one char, "abc"
  DBG_PFU(dc, "%s", "abc");

  // Read it back and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("a", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteEqTmpBufSize)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);

  // Write one char, "abcde"
  const char* str = "abcde";
  ASSERT_EQ(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read it back and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("a", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuOneByteBufWriteGtTmpBufSize)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);

  // Write one char, "abcdef"
  const char* str = "abcdef";
  ASSERT_GT(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read it back and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("a", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteReadRead)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  // Write one char, "a"
  DBG_PFU(dc, "%s", "a");

  // Read it back and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("a", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteReadWriteReadWriteRead)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);
  printf("DbgPfuDbgRead: dst_buf size=%zu begi=%zu endi=%zu\n",
      dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);

  // Write one char, "a"
  DBG_PFU(dc, "%s", "a");

  // Read it back and verify "a" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("a", buf), 0);

  // Write one char, "b"
  DBG_PFU(dc, "%s", "b");

  // Read it back and verify "b" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("b", buf), 0);

  // Write one char, "c"
  DBG_PFU(dc, "%s", "c");

  // Read it back and verify "c" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 1);
  EXPECT_EQ(strcmp("c", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufFillEmpty)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);
  printf("DbgPfuDbgRead: dst_buf size=%zu begi=%zu endi=%zu\n",
      dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);

  // Write "a" then "b"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");

  // Read verify "a"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 1);
  ASSERT_EQ(strcmp("a", buf), 0);

  // Read verify "b"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 1);
  ASSERT_EQ(strcmp("b", buf), 0);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 0);
  ASSERT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufOverFillBy1Empty)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);
  printf("DbgPfuDbgRead: dst_buf size=%zu begi=%zu endi=%zu\n",
      dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);

  // Write "a", "b", "c"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");
  DBG_PFU(dc, "%s", "c");

  // Read verify "b"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 1);
  ASSERT_EQ(strcmp("b", buf), 0);

  // Read verify "c"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 1);
  ASSERT_EQ(strcmp("c", buf), 0);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 0);
  ASSERT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwByteBufOverFillBy2ReadTillEmpty)
{
  char buf[2];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  // Write "a", "b", "c", "d"
  DBG_PFU(dc, "%s", "a");
  DBG_PFU(dc, "%s", "b");
  DBG_PFU(dc, "%s", "c");
  DBG_PFU(dc, "%s", "d");

  // Read verify "c"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 1);
  ASSERT_EQ(strcmp("c", buf), 0);

  // Read verify "d"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 1);
  ASSERT_EQ(strcmp("d", buf), 0);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 0);
  ASSERT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufFillSingleOpReadTillEmpty)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  // Write "ab"
  DBG_PFU(dc, "%s", "ab");

  // Read verify "ab"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 2);
  ASSERT_EQ(strcmp("ab", buf), 0);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 0);
  ASSERT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufOverFillBy1SingleOpReadTillEmpty)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  // Write "ab"
  DBG_PFU(dc, "%s", "abc");

  // Read verify "ab"
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 2);
  ASSERT_EQ(strcmp("ab", buf), 0);

  // Read verify ""
  cnt = dbg_read(dc, buf, sizeof(buf));
  ASSERT_EQ(cnt, 0);
  ASSERT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWrite2Read2)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  // Write one char, "ab"
  const char* str = "ab";
  DBG_PFU(dc, "%s", str);

  // Read it back and verify "ab" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 2);
  EXPECT_EQ(strcmp("ab", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteEqTmpBufSize)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  // Write one char, "abcde"
  const char* str = "abcde";
  ASSERT_EQ(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read it back and verify "ab" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 2);
  EXPECT_EQ(strcmp("ab", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfuTwoByteBufWriteGtTmpBufSize)
{
  char buf[3];
  size_t cnt;
  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);

  // Write one char, "abcdef"
  const char* str = "abcdef";
  ASSERT_GT(strlen(str), DBG_TMP_BUF_SIZE);
  DBG_PFU(dc, "%s", str);

  // Read it back and verify "ab" was written
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 2);
  EXPECT_EQ(strcmp("ab", buf), 0);

  // Read it again, it should now be empty
  cnt = dbg_read(dc, buf, sizeof(buf));
  EXPECT_EQ(cnt, 0);
  EXPECT_EQ(strcmp("", buf), 0);

  dbg_ctx_destroy(dc);
}

//TEST_F(DbgTest, DbgPfuWrite1OverFillSingleOp)
//{
//  char buf[3];
//  size_t cnt;
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(3, 1);
//
//  // Write "1"
//  DBG_PFU(dc, "%s", "1");
//
//  // Write "abc"
//  DBG_PFU(dc, "%s", "abc");
//
//  // Read verify "ab"
//  cnt = dbg_read(dc, buf, sizeof(buf));
//  ASSERT_EQ(cnt, 2);
//  ASSERT_EQ(strcmp("ab", buf), 0);
//
//  // Read verify ""
//  cnt = dbg_read(dc, buf, sizeof(buf));
//  ASSERT_EQ(cnt, 0);
//  ASSERT_EQ(strcmp("", buf), 0);
//
//  dbg_ctx_destroy(dc);
//}
//
//TEST_F(DbgTest, DbgPfuWrite2OverFillSingleOp)
//{
//  char buf[3];
//  size_t cnt;
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(3, 1);
//
//  // Write "12"
//  DBG_PFU(dc, "%s", "12");
//
//  // Write "abc"
//  DBG_PFU(dc, "%s", "abc");
//
//  // Read verify "ab"
//  cnt = dbg_read(dc, buf, sizeof(buf));
//  ASSERT_EQ(cnt, 2);
//  ASSERT_EQ(strcmp("ab", buf), 0);
//
//  // Read verify ""
//  cnt = dbg_read(dc, buf, sizeof(buf));
//  ASSERT_EQ(cnt, 0);
//  ASSERT_EQ(strcmp("", buf), 0);
//
//  dbg_ctx_destroy(dc);
//}
//
////TEST_F(DbgTest, DbgPfuWritesThenReadWrites)
////{
////  char buf[2];
////  size_t cnt;
////  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(3, 1);
////  printf("DbgPfuDbgRead: dst_buf size=%zu begi=%zu endi=%zu\n",
////      dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
////
////  // Write "a" then "b"
////  //printf("DbgPfuDbgRead: write a\n");
////  DBG_PFU(dc, "%s", "a");
////  printf("DbgPfuDbgRead: a cnt=%zu size=%zu begi=%zu endi=%zu\n",
////      cnt, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
////  dump("DbgPfuDbgRead a dst_buf", dc->dst_buf, dc->dst_buf_size);
////  DBG_PFU(dc, "%s", "b");
////  printf("DbgPfuDbgRead: ab cnt=%zu size=%zu begi=%zu endi=%zu\n",
////      cnt, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
////  dump("DbgPfuDbgRead ab dst_buf", dc->dst_buf, dc->dst_buf_size);
////
////  // Read verify "a" write "c"
////  cnt = dbg_read(dc, buf, sizeof(buf));
////  //DBG_PFU(dc, "%s", "c");
////  printf("DbgPfuDbgRead: cnt=%zu size=%zu begi=%zu endi=%zu\n",
////      cnt, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
////  dump("DbgPfuDbgRead", buf, sizeof(buf));
////  ASSERT_EQ(cnt, 1);
////  ASSERT_EQ(strcmp("a", buf), 0);
////
////  // Read verify "b" write "d"
////  cnt = dbg_read(dc, buf, sizeof(buf));
////  DBG_PFU(dc, "%s", "d");
////  //printf("DbgPfuDbgRead: cnt=%zu size=%zu begi=%zu endi=%zu\n",
////  //    cnt, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
////  //dump("DbgPfuDbgRead", buf, sizeof(buf));
////  ASSERT_EQ(cnt, 1);
////  ASSERT_EQ(strcmp("b", buf), 0);
////
////  // Read vierfy "c" and "d"
////  cnt = dbg_read(dc, buf, sizeof(buf));
////  ASSERT_EQ(cnt, 1);
////  ASSERT_EQ(strcmp("c", buf), 0);
////  cnt = dbg_read(dc, buf, sizeof(buf));
////  ASSERT_EQ(cnt, 1);
////  ASSERT_EQ(strcmp("d", buf), 0);
////
////  // Another two reads should both be empty
////  cnt = dbg_read(dc, buf, sizeof(buf));
////  ASSERT_EQ(cnt, 0);
////  ASSERT_EQ(strcmp("", buf), 0);
////  cnt = dbg_read(dc, buf, sizeof(buf));
////  ASSERT_EQ(cnt, 0);
////  ASSERT_EQ(strcmp("", buf), 0);
////
////  dbg_ctx_destroy(dc);
////}
//
////TEST_F(DbgTest, DbgPfuDbgRead)
////{
////  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);
////
////  DBG_PFU(dc, "%s", "a");
////  DBG_PFU(dc, "%s", "b");
////  char buf[2];
////  dbg_read(dc, buf, sizeof(buf));
////  printf("DbgPfuDbgRead: dst_buf size=%zu begi=%zu endi=%zu\n",
////      dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
////  dump("DbgPfuDbgRead", buf, sizeof(buf));
////  EXPECT_EQ(strcmp("a", buf), 0);
////
////  dbg_ctx_destroy(dc);
////}
//
////TEST_F(DbgTest, DbgPfuWrite1byteTo1ByteBuf)
////{
////  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);
////
////  DBG_PFU(dc, "%s", "a");
////  EXPECT_EQ(strcmp("", dbg_get_buf(dc)), 0);
////
////  dbg_ctx_destroy(dc);
////}
////
////TEST_F(DbgTest, DbgPfuWrite2byteTo1ByteBuf)
////{
////  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(1, 1);
////
////  DBG_PFU(dc, "%s", "ab");
////  EXPECT_EQ(strcmp("", dbg_get_buf(dc)), 0);
////
////  dbg_ctx_destroy(dc);
////}
////
////TEST_F(DbgTest, DbgPfuWrite2byteTo2ByteBuf)
////{
////  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(2, 1);
////
////  DBG_PFU(dc, "%s", "ab");
////  EXPECT_EQ(strcmp("a", dbg_get_buf(dc)), 0);
////
////  dbg_ctx_destroy(dc);
////}
////
////TEST_F(DbgTest, DbgPfuTwoSecondTruncated)
////{
////  dbg_ctx_t* dc = dbg_ctx_create_with_dst_buf(3, 1);
////
////  memset(dbg_get_buf(dc), 0xff, 4);
////  DBG_PFU(dc, "%s", "a");
////  dump("DbgPfuTwoSecondTruncated a", dbg_get_buf(dc), 4);
////  printf("DbgPfuTwoSecondTruncated a: %s\n", dbg_get_buf(dc));
////  EXPECT_EQ(strcmp("a", dbg_get_buf(dc)), 0);
////  DBG_PFU(dc, "%s", "bc");
////  dump("DbgPfuTwoSecondTruncated bc", dbg_get_buf(dc), 4);
////  printf("DbgPfuTwoSecondTruncated bc: %s\n", dbg_get_buf(dc));
////  EXPECT_EQ(strcmp("ab", dbg_get_buf(dc)), 0);
////
////  dbg_ctx_destroy(dc);
////}
//
//TEST_F(DbgTest, DbgPsu)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  // Create dc all bits are off
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Validate DBG_PFU still prints and after fclose buffer is valid
//  DBG_PSU(dc, "v=123");
//  fclose(memfile);
//  EXPECT_EQ(strcmp("v=123", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//}
//
//TEST_F(DbgTest, DbgPfnu)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  // Create dc all bits are off
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Validate DBG_PFU still prints and after fclose buffer is valid
//  DBG_PFNU(dc, "v=%d", 123);
//  fclose(memfile);
//  EXPECT_EQ(strcmp("TestBody:  v=123", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//}
//
//TEST_F(DbgTest, DbgPsnu)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  // Create dc all bits are off
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Validate DBG_PFU still prints and after fclose buffer is valid
//  DBG_PSNU(dc, "v=123");
//  fclose(memfile);
//  EXPECT_EQ(strcmp("TestBody:  v=123", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//}
//
//TEST_F(DbgTest, Dbgflush)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Validate DBG_FLUSH can be used instead of fclose
//  DBG_PSU(dc, "123");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("123", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgGbTruthfulness)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  // Use true for setting and see what's returned
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//  dbg_sb(dc, 0, true);
//  EXPECT_EQ(dbg_gb(dc, 0), true);
//  EXPECT_EQ(dbg_gb(dc, 0), 1);
//  EXPECT_TRUE(dbg_gb(dc, 0));
//  EXPECT_TRUE(dbg_gb(dc, 0) != 0);
//
//  // Use non-zero for setting and previous tests should still succeed
//  dbg_sb(dc, 0, ~0);
//  EXPECT_EQ(dbg_gb(dc, 0), true);
//  EXPECT_EQ(dbg_gb(dc, 0), 1);
//  EXPECT_TRUE(dbg_gb(dc, 0));
//  EXPECT_TRUE(dbg_gb(dc, 0) != 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgGbFalsity)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//  dbg_sb(dc, 0, 0);
//  EXPECT_EQ(dbg_gb(dc, 0), false);
//  EXPECT_EQ(dbg_gb(dc, 0), 0);
//  EXPECT_FALSE(dbg_gb(dc, 0));
//  EXPECT_TRUE(dbg_gb(dc, 0) == 0);
//
//  dbg_sb(dc, 0, false);
//  EXPECT_EQ(dbg_gb(dc, 0), false);
//  EXPECT_EQ(dbg_gb(dc, 0), 0);
//  EXPECT_FALSE(dbg_gb(dc, 0));
//  EXPECT_TRUE(dbg_gb(dc, 0) == 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgPf)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Validate nothing is printed after creating
//  // because bit is 0
//  DBG_PF(dc, 0, "%d", 123);
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("", buffer), 0);
//
//  // Now set bit and verify something is printed
//  dbg_sb(dc, 0, true);
//  DBG_PF(dc, 0, "%d", 456);
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("456", buffer), 0);
//
//  // Now clear bit and verify nothing is added
//  dbg_sb(dc, 0, false);
//  DBG_PF(dc, 0, "%d", 789);
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("456", buffer), 0);
//
//  // Now set bit and verify it is now added
//  dbg_sb(dc, 0, true);
//  DBG_PF(dc, 0, "%d\n", 789);
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("456789\n", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgPs)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Validate nothing is printed after creating
//  // because bit is 0
//  DBG_PS(dc, 0, "123");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("", buffer), 0);
//
//  // Now set bit and verify something is printed
//  dbg_sb(dc, 0, true);
//  DBG_PS(dc, 0, "456");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("456", buffer), 0);
//
//  // Now clear bit and verify nothing is added
//  dbg_sb(dc, 0, false);
//  DBG_PS(dc, 0, "789");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("456", buffer), 0);
//
//  // Now set bit and verify it is now added
//  dbg_sb(dc, 0, true);
//  DBG_PS(dc, 0, "789\n");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("456789\n", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgPfn)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Now set bit print with function name and verify
//  dbg_sb(dc, 0, true);
//  DBG_PFN(dc, 0, "%d", 456);
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("TestBody:  456", buffer), 0);
//
//  // Now append somethign using DBG_PF (no function name) and verify
//  DBG_PF(dc, 0, "%d\n", 789);
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("TestBody:  456789\n", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgPsn)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//
//  // Now set bit print with function name and verify
//  dbg_sb(dc, 0, true);
//  DBG_PSN(dc, 0, "456");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("TestBody:  456", buffer), 0);
//
//  // Now append somethign using DBG_PF (no function name) and verify
//  DBG_PS(dc, 0, "789\n");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("TestBody:  456789\n", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgEX)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//  dbg_sb(dc, 0, true);
//
//  // Validate nothing is printed after creating
//  // because bit is 0
//  DBG_E(dc, 0);
//  DBG_X(dc, 0);
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("TestBody:+\nTestBody:-\n", buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgPfeDbgPfx)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//  dbg_sb(dc, 0, true);
//
//  // Validate nothing is printed after creating
//  // because bit is 0
//  DBG_PFE(dc, 0, "Hello, %s\n", "World");
//  DBG_PFX(dc, 0, "%s", "Good bye\n");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("TestBody:+ Hello, World\nTestBody:- Good bye\n",
//        buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgPfeDbgPsx)
//{
//  char buffer[8192];
//
//  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
//  ASSERT_TRUE(memfile != NULL);
//
//  dbg_ctx_t* dc = dbg_ctx_create_with_dst_file(memfile, 1);
//  dbg_sb(dc, 0, true);
//
//  // Validate nothing is printed after creating
//  // because bit is 0
//  DBG_PSE(dc, 0, "Hello, World\n");
//  DBG_PSX(dc, 0, "Good bye\n");
//  DBG_FLUSH(dc);
//  EXPECT_EQ(strcmp("TestBody:+ Hello, World\nTestBody:- Good bye\n",
//        buffer), 0);
//
//  dbg_ctx_destroy(dc);
//  fclose(memfile);
//}
//
//TEST_F(DbgTest, DbgBnoi)
//{
//  EXPECT_EQ(dbg_bnoi(first,0), 0);
//  EXPECT_EQ(dbg_bnoi(first,1), 1);
//  EXPECT_EQ(dbg_bnoi(second,0), 30);
//  EXPECT_EQ(dbg_bnoi(second,1), 31);
//  EXPECT_EQ(dbg_bnoi(second,2), 32);
//  EXPECT_EQ(dbg_bnoi(another,0), 33);
//  EXPECT_EQ(dbg_bnoi(another,1), 34);
//  EXPECT_EQ(bits_size, 30 + 3 + 2);
//}
//
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
