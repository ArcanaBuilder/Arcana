using profiles Debug Release;

COMPILER = g++

@profile Debug
FLAGS    = -std=c++17 -g3 -O0 -Wall -Wextra -pedantic -DDEBUG

@profile Release
FLAGS    = -std=c++17 -Os -Wall -Wextra -DRELEASE

TARGET   = {arc:BUILDDIR}/test
SOURCES  = {arc:SRCDIR}/**.cpp

@map SOURCES
OBJECTS  = {arc:BUILDDIR}

BUILDDIR = build
SRCDIR   = src