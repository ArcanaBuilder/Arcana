using profile Debug Release;

COMPILER = g++

@profile Debug
FLAGS    = -std=c++17 -g3 -O0 -Wall -Wextra -pedantic -DDEBUG

@profile Release
FLAGS    = -std=c++17 -Os -Wall -Wextra -DRELEASE

TARGET   = app

BUILD    = Build
SOURCE   = Source/**.c