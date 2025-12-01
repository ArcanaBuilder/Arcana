using profiles Debug Release;

COMPILER = g++

@profile Debug
FLAGS    = -std=c++17 -g3 -O0 -Wall -Wextra -pedantic -DDEBUG

@profile Release
FLAGS    = -std=c++17 -Os -Wall -Wextra -DRELEASE

TARGET   = arcana

SOURCES  = {SRCDIR}/**.c

@map SOURCES; OBJECTS = {BUILDDIR}/**.o

BUILDDIR = build
SRCDIR   = src