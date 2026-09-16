.PHONY: all build test test-valgrind format format-check lint install uninstall clean

# Configuration selection (override via command line: make CONFIG=xsan)
CONFIG ?= default

PREFIX ?= /usr/local

# Directories (isolated per configuration to prevent object mixing)
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build/$(CONFIG)
BIN_DIR := bin/$(CONFIG)
LOG_DIR := logs
TEST_DIR := tests

# Sanitizer & Compiler Flags
XSAN_FLAGS := -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined
MSAN_FLAGS := -fsanitize=memory -fsanitize-memory-track-origins=2 -stdlib=libc++
#TSAN_FLAGS := -fsanitize=thread

ifeq ($(CONFIG),xsan)
    CONFIG_FLAGS := $(XSAN_FLAGS)
else ifeq ($(CONFIG),msan)
    CONFIG_FLAGS := $(MSAN_FLAGS)
#else ifeq ($(CONFIG),tsan)
#    CONFIG_FLAGS := $(TSAN_FLAGS)
else
    CONFIG_FLAGS :=
endif

OPTIMIZATION_FLAGS := -O3 -fno-omit-frame-pointer
WARNING_FLAGS := -Wall -Wextra -Wconversion -Wsign-conversion -Werror -pedantic

CXX := clang++
CXXFLAGS := -std=c++17 -I$(INC_DIR) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(CONFIG_FLAGS)

LDFLAGS :=
LDLIBS  :=

# Target
TARGET := awakeonlan

MAIN_EXEC := $(BIN_DIR)/$(TARGET)

# Source & Object Discovery
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

NON_MAIN_SRCS := $(filter-out $(SRC_DIR)/main.cpp,$(SRCS))
NON_MAIN_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(NON_MAIN_SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))
TEST_EXECS := $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SRCS))

# ---------------------------------------------------------------------------
# Build Rules
# ---------------------------------------------------------------------------
all: build

build: $(MAIN_EXEC)

# Compile source files (automatically ensures build/ configuration directory exists)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

## Compile test files (automatically ensures build/ configuration directory exists)
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link main executable (automatically ensures bin/ configuration directory exists)
$(MAIN_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Link test executables (automatically ensures bin/ configuration directory exists)
$(TEST_EXECS): $(BIN_DIR)/%: $(BUILD_DIR)/tests/%.o $(NON_MAIN_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# ---------------------------------------------------------------------------
# Test Runners
# ---------------------------------------------------------------------------
test: $(TEST_EXECS)
	@for t in $(TEST_EXECS); do \
	   echo "Running $$t with CONFIG=$(CONFIG)"; \
	   ./$$t || exit 1; \
	done

test-valgrind: $(TEST_EXECS)
	@if [ "$(CONFIG)" != "default" ]; then \
	    echo "Error: Valgrind memcheck cannot run on sanitizer-instrumented binaries (CONFIG=$(CONFIG))." >&2; \
	    echo "Please run: make test-valgrind CONFIG=default" >&2; \
	    exit 1; \
	fi
	@mkdir -p $(LOG_DIR)
	@for t in $(TEST_EXECS); do \
	   base_name=$$(basename $$t); \
	   log_file="$(LOG_DIR)/valgrind_$${base_name}.log"; \
	   echo "Running Valgrind on $$t (Logging to $$log_file)..."; \
	   valgrind \
	       --tool=memcheck \
	       --leak-check=full \
	       --show-leak-kinds=all \
	       --error-exitcode=2 \
	       --track-origins=yes \
	       --verbose \
	       --fair-sched=yes \
	       --log-file="$$log_file" \
	       $$t 192.168.1.0/24 || exit 1; \
	   echo "-> Passed: $$t"; \
	done

# ---------------------------------------------------------------------------
# Code Quality & Formatting
# ---------------------------------------------------------------------------
format:
	bash ./format.sh

format-check:
	bash ./format.sh check

lint:
	bash ./lint.sh

# ---------------------------------------------------------------------------
# Installation
# ---------------------------------------------------------------------------
install: build
	@if [ "$(CONFIG)" != "default" ]; then \
	    echo "Error: install requires CONFIG=default." >&2; \
	    echo "Please run: make install CONFIG=default" >&2; \
	    exit 1; \
	fi
	@echo "If permission is denied, retry with: sudo make install"
	install -d "$(DESTDIR)$(PREFIX)/bin"
	install -m 755 "$(MAIN_EXEC)" "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"

uninstall:
	@echo "If permission is denied, retry with: sudo make uninstall"
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"

# ---------------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------------
clean:
	rm -rf build bin logs
