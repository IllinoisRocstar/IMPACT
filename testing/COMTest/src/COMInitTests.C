#include <fstream>
#include <iostream>
#include "COM_base.hpp"
#include "com_c++.hpp"
#include "gtest/gtest.h"

// Global variables used to pass arguments to the tests
char** ARGV;
int ARGC;

// Test fixture which tests COM. Derived from the Google test primer

class COMInitTest : public ::testing::Test {
 protected:
  COMInitTest() {}
  void SetUp() {
    // MPI_Init(&ARGC,&ARGV);
    COM_init(&ARGC, &ARGV);
  }
  void TearDown() {
    COM_finalize();
    ASSERT_EQ(nullptr, COM::COM_base::get_com())
        << "COM was not destroyed correctly" << std::endl
        << "pointer to comm is not a nullptr" << std::endl;
    std::cout << "The pointer to Com is a nullptr" << std::endl
              << "Com has been finalized succesfully" << std::endl;
    // MPI_Finalize();
  }
};

TEST_F(COMInitTest, comInitializedTest) {
  /* int isCOMinitialized = COM_initialized();
        bool com_initialized_pass = (isCOMinitialized <= 0);

        if(com_initialized_pass)
                isCOMinitialized = (COM_initialized() > 0); */
  EXPECT_TRUE(COM_initialized())
      << "COM_initialized returns false" << std::endl;
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  ARGC = argc;
  ARGV = argv;
  return RUN_ALL_TESTS();
}