CXX=g++
CXXFLAGS=-Wall -std=c++20
GTEST_LIBS = -lgtest -lgtest_main -pthread

BUILD_DIR=build
SRC_DIR=src
TEST_DIR=tests

SRCS=$(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS=$(wildcard $(TEST_DIR)/*.cpp)
TEST_SRCS+=$(filter-out $(SRC_DIR)/main.cpp, $(SRCS))

OBJS=$(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

EXEC=imapcl

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC) -lssl -lcrypto

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests:
	$(CXX) $(TEST_SRCS) -o $(TEST_DIR)/basic $(GTEST_LIBS)

run-tests:
	./tests/basic

clean:
	rm -rf $(BUILD_DIR)
	rm imapcl

debug: CXXFLAGS += -g -O0
debug: all

.PHONY: all clean tests run-tests
