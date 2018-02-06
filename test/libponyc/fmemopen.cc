#include <gtest/gtest.h>
#include <platform.h>
#include <string.h>

class FmemOpenTest: public testing::Test
{};

TEST_F(FmemOpenTest, Fmemopen)
{
#ifndef _MSC_VER
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
#endif
}

TEST_F(FmemOpenTest, Fseek)
{
#ifndef _MSC_VER
  char buffer[8192];

  FILE* memfile = fmemopen(buffer, sizeof(buffer), "w+");
  ASSERT_TRUE(memfile != NULL);

  // Write something
  fprintf(memfile, "123");
  EXPECT_EQ(ftell(memfile), 3);
  fflush(memfile);
  EXPECT_EQ(strcmp("123", buffer), 0);

  // fseek back to beginning, write and verify.
  // NOTE: The behavior of not writing a zero when flushing
  // means using fseek to reuse a buffer is not advisible
  // in general.
  fseek(memfile, 0, SEEK_SET);
  EXPECT_EQ(ftell(memfile), 0);
  fprintf(memfile, "45");
  fflush(memfile);
  EXPECT_EQ(strcmp("453", buffer), 0);

  fclose(memfile);
#endif
}
