#include <gtest/gtest.h>
#include <platform.h>

#define DBG_ENABLED true
#include "../../src/libponyc/dbg/dbg.h"

class DbgBitsTest : public testing::Test
{};

TEST_F(DbgBitsTest, TestDcBi)
{
  ASSERT_EQ(dc_bi(0,0), 0);
  ASSERT_EQ(dc_bi(0,1), 1);
  ASSERT_EQ(dc_bi(1,0), 1);
  ASSERT_EQ(dc_bi(1,1), 2);
}

TEST_F(DbgBitsTest, TestDcBni)
{
  ASSERT_EQ(dc_bni(first,0), 0);
  ASSERT_EQ(dc_bni(first,1), 1);
  ASSERT_EQ(dc_bni(dummy,0), 32);
  ASSERT_EQ(dc_bni(dummy,1), 33);
}

TEST_F(DbgBitsTest, TestDcBitsArrayIdx)
{
  ASSERT_EQ(_DC_BITS_ARRAY_IDX(0), 0);
  ASSERT_EQ(_DC_BITS_ARRAY_IDX(31), 0);
  ASSERT_EQ(_DC_BITS_ARRAY_IDX(32), 1);
  ASSERT_EQ(_DC_BITS_ARRAY_IDX(63), 1);
  ASSERT_EQ(_DC_BITS_ARRAY_IDX(64), 2);
  ASSERT_EQ(_DC_BITS_ARRAY_IDX(95), 2);
}

TEST_F(DbgBitsTest, TestDcBitMask)
{
  ASSERT_EQ(_DC_BIT_MASK(0),  0x00000001);
  ASSERT_EQ(_DC_BIT_MASK(1),  0x00000002);
  ASSERT_EQ(_DC_BIT_MASK(31), 0x80000000);
  ASSERT_EQ(_DC_BIT_MASK(32), 0x00000001);
  ASSERT_EQ(_DC_BIT_MASK(63), 0x80000000);
  ASSERT_EQ(_DC_BIT_MASK(64), 0x00000001);
  ASSERT_EQ(_DC_BIT_MASK(95), 0x80000000);
}

TEST_F(DbgBitsTest, TestDcInitDestroy)
{
  dbg_ctx_t* dc = dc_create(NULL, 2);

  // Verify data structure
  ASSERT_EQ(dc->bits[0], 0);
  ASSERT_TRUE(dc->file == NULL);
  
  dc_destroy(dc);

  dc = dc_create(stdout, 2);

  // Verify data structure
  ASSERT_TRUE(dc->file != NULL);
  ASSERT_EQ(dc->bits[0], 0);
  ASSERT_EQ(dc->file, stdout);
  
  dc_destroy(dc);
}

TEST_F(DbgBitsTest, TestWalkingOneBit)
{
  const uint32_t num_bits = 64; // Assume power of 2
  dbg_ctx_t* dc = dc_create(NULL, num_bits);

  // Verify all bits are zero
  for(uint32_t bi = 0; bi < num_bits; bi++)
  {
    ASSERT_FALSE(dc_gb(dc, bi));
  }

  // Verify bits array is zero
  for(uint32_t i = 0; i < (num_bits + 31) / 32; i++)
  {
    ASSERT_EQ(dc->bits[i], 0);
  }

  // Walking one bit
  for(uint32_t bi = 0; bi < num_bits; bi++)
  {
    bool v = dc_gb(dc, bi);
    ASSERT_FALSE(v);
    dc_sb(dc, bi, 1);
    v = dc_gb(dc, bi);
    ASSERT_TRUE(v);
    
    // Verify only one bit is set
    for(uint32_t cbi = 0; cbi < num_bits; cbi++)
    {
      bool v = dc_gb(dc, cbi);
      if (cbi == bi)
        ASSERT_TRUE(v);
      else
        ASSERT_FALSE(v);
    }

    // Verify only one bit is set using bits array
    uint32_t bits_array_idx = bi / 32;
    uint32_t bits_mask = 1 << (bi % 32);
    for(uint32_t i = 0; i < (num_bits + 31) / 32; i++)
    {
      if(i == bits_array_idx)
        ASSERT_EQ(dc->bits[i], bits_mask);
      else
        ASSERT_EQ(dc->bits[i], 0);
    }
    dc_sb(dc, bi, 0);
  }

  dc_destroy(dc);
}

TEST_F(DbgBitsTest, TestWalkingTwoBits)
{
  const uint32_t num_bits = 64; // Assume power of 2
  dbg_ctx_t* dc = dc_create(NULL, num_bits);

  // Verify all bits are zero
  for(uint32_t bi = 0; bi < num_bits; bi++)
  {
    ASSERT_FALSE(dc_gb(dc, bi));
  }

  // Verify bits array is zero
  for(uint32_t i = 0; i < (num_bits + 31) / 32; i++)
  {
    ASSERT_EQ(dc->bits[i], 0);
  }

  // Walking two bits
  dc_sb(dc, 0, 1);
  for(uint32_t bi = 1; bi < num_bits - 1; bi++)
  {
    // Get first of the pair it should be 1
    bool v = dc_gb(dc, bi - 1);
    ASSERT_TRUE(v);

    // Get second bit it should be 0
    v = dc_gb(dc, bi);
    ASSERT_FALSE(v);
    
    // Set second bit
    dc_sb(dc, bi, 1);

    // Verify two bits are set
    for(uint32_t cbi = 0; cbi < num_bits; cbi++)
    {
      bool v = dc_gb(dc, cbi);
      if (cbi == (bi - 1))
        ASSERT_TRUE(v);
      else if (cbi == bi)
        ASSERT_TRUE(v);
      else
        ASSERT_FALSE(v);
    }
    
    // Clear first bit
    dc_sb(dc, bi - 1, 0);
  }

  dc_destroy(dc);
}

TEST_F(DbgBitsTest, TestDcReadWriteBitsOfDummy)
{
  dbg_ctx_t* dc = dc_create(NULL, dc_bni(dummy, 2));

  // Get bit index of dummy[0] and dummy[1] they should be adjacent
  uint32_t bi0 = dc_bni(dummy, 0);
  uint32_t bi1 = dc_bni(dummy, 1);
  ASSERT_EQ(bi0 + 1, bi1);

  // Initially they should be zero
  bool o0 = dc_gb(dc, bi0);
  bool o1 = dc_gb(dc, bi1);
  ASSERT_FALSE(o0);
  ASSERT_FALSE(o1);

  // Write new values and verify they changed
  dc_sb(dc, bi0, !o0);
  dc_sb(dc, bi1, !o1);
  ASSERT_EQ(dc_gb(dc, bi0), !o0);
  ASSERT_EQ(dc_gb(dc, bi1), !o1);

  // Restore original values
  dc_sb(dc, bi0, o0);
  dc_sb(dc, bi1, o1);
  ASSERT_EQ(dc_gb(dc, bi0), o0);
  ASSERT_EQ(dc_gb(dc, bi1), o1);

  dc_destroy(dc);
}
