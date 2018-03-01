#include <gtest/gtest.h>
#include <platform.h>

#define DBG_ENABLED true
#include "../../../src/libponyrt/dbg/dbg.h"
#include "../../../src/libponyrt/dbg/dbg_util.h"

// Include file and mem implementations to validate both can be
// used simultaneously.
#include "../../../src/libponyrt/dbg/dbg_ctx_impl_file.h"
#include "../../../src/libponyrt/dbg/dbg_ctx_impl_mem.h"

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
  DBG_BITS_SIZE(bit_count, another)
};

class DbgTest : public testing::Test
{};

TEST_F(DbgTest, DbgDoEmpty)
{
  // Should compile
  _DBG_DO();
}

TEST_F(DbgTest, DoSingleStatement)
{
  char buffer[10];
  _DBG_DO(strcpy(buffer, "Hi"));
  EXPECT_STREQ(buffer, "Hi");
}

TEST_F(DbgTest, DoMultiStatement)
{
  char buffer[10];
  _DBG_DO(strcpy(buffer, "Hi"); strcat(buffer, ", Bye"));
  EXPECT_STREQ(buffer, "Hi, Bye");
}

TEST_F(DbgTest, BitsArrayIdx)
{
  if(sizeof(size_t) == 4)
  {
    EXPECT_EQ(dbg_bits_array_idx(0), 0);
    EXPECT_EQ(dbg_bits_array_idx(31), 0);
    EXPECT_EQ(dbg_bits_array_idx(32), 1);
    EXPECT_EQ(dbg_bits_array_idx(63), 1);
    EXPECT_EQ(dbg_bits_array_idx(64), 2);
    EXPECT_EQ(dbg_bits_array_idx(95), 2);
    EXPECT_EQ(dbg_bits_array_idx(96), 3);
  } else if(sizeof(size_t) == 8) {
    EXPECT_EQ(dbg_bits_array_idx(0), 0);
    EXPECT_EQ(dbg_bits_array_idx(63), 0);
    EXPECT_EQ(dbg_bits_array_idx(64), 1);
    EXPECT_EQ(dbg_bits_array_idx(127), 1);
    EXPECT_EQ(dbg_bits_array_idx(128), 2);
    EXPECT_EQ(dbg_bits_array_idx(191), 2);
    EXPECT_EQ(dbg_bits_array_idx(192), 3);
  } else {
    EXPECT_TRUE(false);
  }
}

TEST_F(DbgTest, BitMask)
{
  if(sizeof(size_t) == 4)
  {
    EXPECT_EQ(dbg_bit_mask(0),  0x00000001);
    EXPECT_EQ(dbg_bit_mask(1),  0x00000002);
    EXPECT_EQ(dbg_bit_mask(31), 0x80000000);
    EXPECT_EQ(dbg_bit_mask(32), 0x00000001);
    EXPECT_EQ(dbg_bit_mask(63), 0x10000000);
    EXPECT_EQ(dbg_bit_mask(64), 0x00000001);
    EXPECT_EQ(dbg_bit_mask(95), 0x80000000);
  } else if(sizeof(size_t) == 8) {
    EXPECT_EQ(dbg_bit_mask(0),  0x0000000000000001);
    EXPECT_EQ(dbg_bit_mask(1),  0x0000000000000002);
    EXPECT_EQ(dbg_bit_mask(31), 0x0000000080000000);
    EXPECT_EQ(dbg_bit_mask(32), 0x0000000100000000);
    EXPECT_EQ(dbg_bit_mask(63), 0x8000000000000000);
    EXPECT_EQ(dbg_bit_mask(64), 0x0000000000000001);
    EXPECT_EQ(dbg_bit_mask(95), 0x0000000080000000);
  } else {
    EXPECT_TRUE(false);
  }
}

TEST_F(DbgTest, Bnoi)
{
  EXPECT_EQ(dbg_bnoi(first,0), 0);
  EXPECT_EQ(dbg_bnoi(first,1), 1);
  EXPECT_EQ(dbg_bnoi(second,0), 30);
  EXPECT_EQ(dbg_bnoi(second,1), 31);
  EXPECT_EQ(dbg_bnoi(second,2), 32);
  EXPECT_EQ(dbg_bnoi(another,0), 33);
  EXPECT_EQ(dbg_bnoi(another,1), 34);
  EXPECT_EQ(bit_count, 30 + 3 + 2);
}

TEST_F(DbgTest, ReadWriteBitsOfSecond)
{
  dbg_ctx_t* dc = dbg_ctx_create(bit_count);

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

TEST_F(DbgTest, CreateDestroy)
{
  // If 0 bits then no dcc->bits array
  dbg_ctx_t* dc = dbg_ctx_create(0);
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;
  ASSERT_TRUE(dcc->bits == NULL);
  EXPECT_EQ(dcc->id, DBG_CTX_ID_COMMON);

  // If we try to set bit it shouldn't fault and getting should be false
  dbg_sb(dc, 0, 1);
  EXPECT_EQ(dbg_gb(dc, 0), false);
  dbg_ctx_destroy(dc);

  // Test allocating 1
  dc = dbg_ctx_create(1);
  dcc = (dbg_ctx_common_t*)dc;
  ASSERT_TRUE(dcc->bits != NULL);
  EXPECT_EQ(dcc->bits[0], 0);
  dbg_sb(dc, 0, 1);
  EXPECT_EQ(dbg_gb(dc, 0), true);

  // If we try to set bit past end it shouldn't fault
  // and getting should be false
  dbg_sb(dc, 1, 1);
  EXPECT_EQ(dbg_gb(dc, 1), false);

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, BitsInitToZero)
{
  const uint32_t num_bits = 65;
  dbg_ctx_t* dc = dbg_ctx_create(num_bits);
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;

  // Verify all bits are zero
  for(uint32_t bi = 0; bi < num_bits; bi++)
  {
    EXPECT_FALSE(dbg_gb(dc, bi));
  }

  // Verify bits array is zero
  for(uint32_t i = 0; i < (num_bits + 31) / 32; i++)
  {
    EXPECT_EQ(dcc->bits[i], 0);
  }

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, WalkingOneBit)
{
  const uint32_t num_bits = 29;
  dbg_ctx_t* dc = dbg_ctx_create(num_bits);
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;

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
        EXPECT_EQ(dcc->bits[i], bits_mask);
      else
        EXPECT_EQ(dcc->bits[i], 0);
    }
    dbg_sb(dc, bi, 0);
  }

  dbg_ctx_destroy(dc);
}

TEST_F(DbgTest, WalkingTwoBits)
{
  const uint32_t num_bits = 147;
  dbg_ctx_t* dc = dbg_ctx_create(num_bits);

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

#if !defined(PLATFORM_IS_WINDOWS) && !defined(PLATFORM_IS_MACOSX)
TEST_F(DbgTest, MemAndFile)
{
  char buffer[0x100];
  char expected[0x100];
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  dbg_ctx_t* dcf = dbg_ctx_impl_file_create(1, memfile);
  ASSERT_TRUE(dcf != NULL);
  dbg_ctx_t* dcm = dbg_ctx_impl_mem_create(1, 0x1000, 0x1000);
  ASSERT_TRUE(dcm != NULL);

  // Test file implementation
  snprintf(expected, sizeof(expected), "TestBody:%-4d hi dcf\n", __LINE__ + 1);
  dbg_printf(DBG_LOC(dcf), "hi dcf\n");
  fclose(memfile);

  EXPECT_STREQ(expected, buffer);

  // Test mem implementation
  snprintf(expected, sizeof(expected), "TestBody:%-4d hi dcm\n", __LINE__ + 1);
  dbg_printf(DBG_LOC(dcm), "hi dcm\n");

  size_t len = dbg_read(dcm, buffer, sizeof(buffer), sizeof(buffer));
  EXPECT_EQ(strlen(expected), len);
  EXPECT_STREQ(expected, buffer);

  dbg_ctx_destroy(dcf);
  dbg_ctx_destroy(dcm);
}
#endif
