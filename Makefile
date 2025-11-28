CXX      := g++
CXXFLAGS := -std=c++17 -g3 -O0 -Wall -Wextra -DDEBUG -Iinclude/core -Iinclude/generator -Iinclude/parser -Iinclude/util

TARGET   := build/bin/arcana

SRCS := \
    $(wildcard src/*.cpp) \
    $(wildcard src/core/*.cpp) \
    $(wildcard src/generator/*.cpp) \
    $(wildcard src/parser/*.cpp) \
    $(wildcard src/util/*.cpp)

OBJS := $(patsubst src/%.cpp,build/%.o,$(SRCS))

.PHONY: all clean

all: $(TARGET)


$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

install:
	@mv $(TARGET) /usr/bin

clean:
	rm -rf build
