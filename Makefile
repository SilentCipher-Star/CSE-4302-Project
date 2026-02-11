#**************************************************************************************************
#
#   Makefile for Terminal Project
#
#**************************************************************************************************

.PHONY: all clean

# Project Name
PROJECT_NAME       ?= game

# Define default C++ compiler
CC = g++

# Define default make program
MAKE = make

# Define compiler flags
CFLAGS += -Wall -std=c++14 -D_DEFAULT_SOURCE

# Build mode: DEBUG or RELEASE
BUILD_MODE ?= RELEASE

ifeq ($(BUILD_MODE),DEBUG)
    CFLAGS += -g -O0
else
    CFLAGS += -s -O1
endif

# Define a recursive wildcard function
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# Define all source files required
SRC_DIR = src

# Define build directory
BUILD_DIR = build

# Define all source files
SRC = $(call rwildcard, src, *.cpp)

# Default target entry
all: $(PROJECT_NAME)

# Project target defined by PROJECT_NAME
$(PROJECT_NAME): $(SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/$(PROJECT_NAME) $(SRC) $(CFLAGS)
	cp users.txt $(BUILD_DIR)/

# Clean everything
clean:
	rm -rf $(BUILD_DIR)
	@echo Cleaning done
