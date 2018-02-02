#include <gtest/gtest.h>
#include <platform.h>
#include <string.h>

#define DBG_ENABLED true
#include "../../src/libponyc/dbg/dbg.h"

class DbgTest : public testing::Test
{};

TEST_F(DbgTest, TestFmemopen)
{
  char buffer[8192];
  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  // Using "w+" we will have a 0 in the buffer
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
