# Makefile for compiling decode_mesytec_vme_data.cc

# Compiler
CXX ?= g++

# Compiler flags
CXXFLAGS ?= -Wall -O2

# Default target
all: decode_mesytec_vme_data

# Rule to build the target
decode_mesytec_vme_data: decode_mesytec_vme_data.o
	$(CXX) $(CXXFLAGS) -o decode_mesytec_vme_data decode_mesytec_vme_data.o

# Rule to build object files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f decode_mesytec_vme_data.o decode_mesytec_vme_data

.PHONY: all clean
