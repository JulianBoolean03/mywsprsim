# JTEncode Library Makefile
CC = gcc
CXX = g++
CFLAGS = -O2 -Wall -fPIC -I.
CXXFLAGS = -O2 -Wall -fPIC -std=c++11 -I.

# Library name
LIBNAME = libjtencode.a

# Source files
C_SOURCES = crc14.c nhash.c
CXX_SOURCES = JTEncode.cpp encode_rs_int.cpp init_rs_int.cpp

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS)

# Default target
all: $(LIBNAME)

# Build library
$(LIBNAME): $(OBJECTS)
	ar rcs $@ $^
	cp $@ ../

# C source compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# C++ source compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(LIBNAME)
	rm -f ../$(LIBNAME)

# Install (copy to parent directory)
install: $(LIBNAME)
	cp $(LIBNAME) ../

.PHONY: all clean install
