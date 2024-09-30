#include <gtest/gtest.h>
#include "../src/argparser.hpp"
#include <iostream>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(ArgParser, NonNumberPort) {
    char *argv[] = {
        (char *)"imapcl", 
        (char *)"-p",     
        (char *)"asdqerq",
        nullptr           
    };

    int argc = 3;
    
    ArgParser args;
    EXPECT_THROW({
            args.parse(argv, argc);
      }, std::invalid_argument);
}