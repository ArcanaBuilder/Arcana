CXX      := g++
CXXFLAGS := -std=c++17 -g3 -O0 -Wall -Wextra -DDEBUG -Iinclude/common -Iinclude/core -Iinclude/parser -Iinclude/parser/util -Iinclude/core/util

TARGET   := build/bin/arcana

SRCS := 								\
    $(wildcard src/*.cpp) 				\
    $(wildcard src/core/*.cpp) 			\
    $(wildcard src/core/util/*.cpp) 	\
    $(wildcard src/parser/*.cpp) 		\
    $(wildcard src/parser/util/*.cpp) 

OBJS := $(patsubst src/%.cpp,build/%.o,$(SRCS))

.PHONY: all clean

all: $(TARGET)


$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: clean all
	@cp $(TARGET) /usr/bin

clean:
	rm -rf build
