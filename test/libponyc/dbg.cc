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

TEST_F(DbgTest, Fmemopen)
{
  char buffer[8192];

  // Create memory file use "w+" so the first byte is a 0
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);
  EXPECT_EQ(strlen(buffer), 0);

  // Write some data and flush to see the contents
  fprintf(memfile, "hi");
  fflush(memfile);
  EXPECT_EQ(strlen(buffer), 2);
  EXPECT_EQ(strcmp(buffer, "hi"), 0);

  // If we don't flush then the data may or may not
  // be in the buffer so we can't expect anything.
  fprintf(memfile, "1");
  fprintf(memfile, "2");
  //EXPECT_NE(strcmp(buffer, "hi12"), 0);

  // Closing will flush and close now we can
  // expect the data to be in the buffer
  int r = fclose(memfile);
  EXPECT_EQ(r, 0);
  EXPECT_EQ(strlen(buffer), 4);
  EXPECT_EQ(strcmp(buffer, "hi12"), 0);
}

TEST_F(DbgTest, DbgDoEmpty)
{
  // Should compile
  _DBG_DO();
}

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

TEST_F(DbgTest, DbgInitDestroy)
{
  // Verify data structure
  dbg_ctx_t* dc = dbg_ctx_create(NULL, 1);
  ASSERT_TRUE(dc->bits != NULL);
  EXPECT_EQ(dc->bits[0], 0);
  EXPECT_TRUE(dc->file == NULL);
  dbg_ctx_destroy(dc);

  dc = dbg_ctx_create(stdout, 1);
  ASSERT_TRUE(dc->bits != NULL);
  EXPECT_EQ(dc->bits[0], 0);
  EXPECT_EQ(dc->file, stdout);
  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgCtxBitsInitToZero)
{
  const uint32_t num_bits = 65;
  dbg_ctx_t* dc = dbg_ctx_create(NULL, num_bits);

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

TEST_F(DbgTest, TestWalkingOneBit)
{
  const uint32_t num_bits = 29;
  dbg_ctx_t* dc = dbg_ctx_create(NULL, num_bits);

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

TEST_F(DbgTest, TestWalkingTwoBits)
{
  const uint32_t num_bits = 147;
  dbg_ctx_t* dc = dbg_ctx_create(NULL, num_bits);

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
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  // Create dc all bits are off
  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);

  // Validate DBG_PFU still prints and after fclose buffer is valid
  DBG_PFU(dc, "123");
  fclose(memfile);
  EXPECT_EQ(strcmp("123", buffer), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, DbgPfnu)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  // Create dc all bits are off
  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);

  // Validate DBG_PFU still prints and after fclose buffer is valid
  DBG_PFNU(dc, "123");
  fclose(memfile);
  EXPECT_EQ(strcmp("TestBody:  123", buffer), 0);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, Dbgflush)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);

  // Validate DBG_FLUSH can be used instead of fclose
  DBG_PFU(dc, "123");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("123", buffer), 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
}

TEST_F(DbgTest, TestFseek)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);

  // Write something
  DBG_PFU(dc, "123");
  EXPECT_EQ(ftell(memfile), 3);
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("123", buffer), 0);

  // fseek back to beginning, write and verify.
  // NOTE: The behavior of not writing a zero when flushing
  // means using fseek to reuse a buffer is not advisible
  // in general.
  fseek(memfile, 0, SEEK_SET);
  EXPECT_EQ(ftell(memfile), 0);
  DBG_PFU(dc, "45");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("453", buffer), 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
}

TEST_F(DbgTest, TestTruthfulnessOfDbgGb)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  // Use true for setting and see what's returned
  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);
  dbg_sb(dc, 0, true);
  EXPECT_EQ(dbg_gb(dc, 0), true);
  EXPECT_EQ(dbg_gb(dc, 0), 1);
  EXPECT_TRUE(dbg_gb(dc, 0));
  EXPECT_TRUE(dbg_gb(dc, 0) != 0);

  // Use non-zero for setting and previous tests should still succeed
  dbg_sb(dc, 0, ~0);
  EXPECT_EQ(dbg_gb(dc, 0), true);
  EXPECT_EQ(dbg_gb(dc, 0), 1);
  EXPECT_TRUE(dbg_gb(dc, 0));
  EXPECT_TRUE(dbg_gb(dc, 0) != 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
}

TEST_F(DbgTest, TestFalsityOfDbgGb)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);
  dbg_sb(dc, 0, 0);
  EXPECT_EQ(dbg_gb(dc, 0), false);
  EXPECT_EQ(dbg_gb(dc, 0), 0);
  EXPECT_FALSE(dbg_gb(dc, 0));
  EXPECT_TRUE(dbg_gb(dc, 0) == 0);

  dbg_sb(dc, 0, false);
  EXPECT_EQ(dbg_gb(dc, 0), false);
  EXPECT_EQ(dbg_gb(dc, 0), 0);
  EXPECT_FALSE(dbg_gb(dc, 0));
  EXPECT_TRUE(dbg_gb(dc, 0) == 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
}

TEST_F(DbgTest, DbgPf)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);

  // Validate nothing is printed after creating
  // because bit is 0
  DBG_PF(dc, 0, "123");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("", buffer), 0);

  // Now set bit and verify something is printed
  dbg_sb(dc, 0, true);
  DBG_PF(dc, 0, "456");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("456", buffer), 0);

  // Now clear bit and verify nothing is added
  dbg_sb(dc, 0, false);
  DBG_PF(dc, 0, "789");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("456", buffer), 0);

  // Now set bit and verify it is now added
  dbg_sb(dc, 0, true);
  DBG_PF(dc, 0, "789\n");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("456789\n", buffer), 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
}

TEST_F(DbgTest, DbgPfn)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);

  // Now set bit print with function name and verify
  dbg_sb(dc, 0, true);
  DBG_PFN(dc, 0, "456");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("TestBody:  456", buffer), 0);

  // Now append somethign using DBG_PF (no function name) and verify
  DBG_PF(dc, 0, "789\n");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("TestBody:  456789\n", buffer), 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
}

TEST_F(DbgTest, DbgEX)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);
  dbg_sb(dc, 0, true);

  // Validate nothing is printed after creating
  // because bit is 0
  DBG_E(dc, 0);
  DBG_X(dc, 0);
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("TestBody:+\nTestBody:-\n", buffer), 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
}

TEST_F(DbgTest, DbgPfeDbgPfx)
{
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  dbg_ctx_t* dc = dbg_ctx_create(memfile, 1);
  dbg_sb(dc, 0, true);

  // Validate nothing is printed after creating
  // because bit is 0
  DBG_PFE(dc, 0, "Hello, %s\n", "World");
  DBG_PFX(dc, 0, "Good bye\n");
  DBG_FLUSH(dc);
  EXPECT_EQ(strcmp("TestBody:+ Hello, World\nTestBody:- Good bye\n",
        buffer), 0);

  dbg_ctx_destroy(dc);
  fclose(memfile);
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

TEST_F(DbgTest, DbgReadWriteBitsOfSecond)
{
  dbg_ctx_t* dc = dbg_ctx_create(NULL, bits_size);

  // Get bit index of second[0] and second[1] they should be adjacent
  uint32_t bi0 = dbg_bnoi(second, 0);
  uint32_t bi1 = dbg_bnoi(second, 1);
  EXPECT_EQ(bi0 + 1, bi1);

  // Initially they should be zero
  bool o0 = dbg_gb(dc, bi0);
  bool o1 = dbg_gb(dc, bi1);
  EXPECT_FALSE(o0);
  EXPECT_FALSE(o1);

  // Write new values and verify they changed
  dbg_sb(dc, bi0, !o0);
  dbg_sb(dc, bi1, !o1);
  EXPECT_EQ(dbg_gb(dc, bi0), !o0);
  EXPECT_EQ(dbg_gb(dc, bi1), !o1);

  // Restore original values
  dbg_sb(dc, bi0, o0);
  dbg_sb(dc, bi1, o1);
  EXPECT_EQ(dbg_gb(dc, bi0), o0);
  EXPECT_EQ(dbg_gb(dc, bi1), o1);

  dbg_ctx_destroy(dc);
}
