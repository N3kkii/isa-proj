CXX=g++
EXEC=imapcl
CXXFLAGS=-Wall -std=c++20
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

all: $(EXEC)
	$(CXX) -o $(EXEC) $(OBJS)

$(EXEC): $(OBJS)

clean:
	rm $(OBJS)

	
.PHONY: clean