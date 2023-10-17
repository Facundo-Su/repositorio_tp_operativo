# Libraries
LIBS=shared commons readline

# Custom libraries' paths
SHARED_LIBPATHS=
STATIC_LIBPATHS=../shared

# Compiler flags
CDEBUG=-g -Wall -DDEBUG -fcommon -fdiagnostics-color=always
CRELEASE=-O3 -Wall -DNDEBUG -fcommon

# Arguments when executing with start, memcheck or helgrind
ARGS=

# Valgrind flags
MEMCHECK_FLAGS=--track-origins=yes --log-file="memcheck.log"
HELGRIND_FLAGS=--log-file="helgrind.log"

# Source files (*.c) to be excluded from tests compilation
TEST_EXCLUDE=src/main.c
