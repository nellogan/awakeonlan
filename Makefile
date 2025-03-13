.PHONY: all build test build-system-test test-xsan test-msan test-tsan test-sanitizers test-memcheck test-helgrind \
test-valgrind clean clean-all

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

PROJ_NAME := awakeonlan
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin
TEST_DIR := test
LOG_DIR := log

_CREATE_BUILD_DIR := $(shell mkdir -p $(BUILD_DIR))
_CREATE_BIN_DIR := $(shell mkdir -p $(BIN_DIR))
_CREATE_LOG_DIR := $(shell mkdir -p $(LOG_DIR))

#CXX := g++
CXX := clang++
OPT := -O2
WARNING_FLAGS := -Wall -Wextra -Werror -pedantic
CXXFLAGS := -I$(INC_DIR) $(OPT) $(WARNING_FLAGS)
SAN_FLAG ?=

# gcc/g++ does not support memory sanitizer.
# By default undefined sanitizer does not halt execution upon error so '-fno-sanitize-recover=undefined' is added.
# Address, leak, and undefined sanitizer may be combined but memory and thread sanitizers cannot.
# NB: -stdlib=libc++ must be included for memory sanitizer (sudo apt install libc++-dev if missing) to work properly,
# else false positives will be abundant. Memory sanitizer requires all code (including c++ standard libraries) to be
# instrumented. If the apt installed libc++ was not instrumented with memory sanitizer download clangs source and
# compile it. See:
#	https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
#	https://github.com/google/sanitizers/issues/1815
ifeq ($(SAN_FLAG), xsan)
	CXXFLAGS += -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined
else ifeq ($(SAN_FLAG), msan)
	ifeq ($(CXX), clang++)
	    CXXFLAGS += -fsanitize=memory -stdlib=libc++
	else
        $(warning "Warning: gcc/g++ does not support memory sanitizer. Tests will be performed without memory sanitizer.")
	endif
else ifeq ($(SAN_FLAG), tsan)
	CXXFLAGS += -fsanitize=thread
endif

LDFLAGS :=
LDLIBS :=

TARGET := $(PROJ_NAME)
MAIN_NAME := main.cpp
MAIN_EXEC := $(BIN_DIR)/$(TARGET)
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
NON_MAIN_SRCS := $(filter-out $(SRC_DIR)/$(MAIN_NAME), $(SRCS))
NON_MAIN_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(NON_MAIN_SRCS))

TEST_DIR_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_SRCS := $(notdir $(NON_MAIN_SRCS) $(TEST_DIR_SRCS))
TEST_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(TEST_SRCS))
TEST_EXECS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/%, $(TEST_DIR_SRCS))
SYSTEM_TEST_NAME := system_test
SYSTEM_TEST_EXEC := $(BIN_DIR)/$(SYSTEM_TEST_NAME)

all: build

build: $(MAIN_EXEC)

build-system-test: build $(SYSTEM_TEST_EXEC)

# Compile source files.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link main executable.
$(MAIN_EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

# Compile test files.
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link test executables.
$(TEST_EXECS): $(BIN_DIR)/%: $(BUILD_DIR)/%.o $(NON_MAIN_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

test: $(TEST_EXECS)
	@for test in $(TEST_EXECS); do \
		echo "Running $$test"; \
        $$test ; \
        FAIL=$$? ; \
        if [ $$FAIL -ne 0 ]; then \
            echo FAILED test: $$test ; \
            exit 2 ; \
        fi ; \
	done

# NOTE: Some older versions of sanitizers (e.g. what the compiler may have been built with) have a bug regarding
# ASLR bit counts exceeding 28. Most modern Linux kernels will set mmap_rnd_bits to 32. Will probably encounter this
# if using Ubuntu 22.04's apt installed g++ or clang++.
# The systemd work around: 'sudo sysctl -w vm.mmap_rnd_bits=28'
test-xsan:
	make clean-all
	make SAN_FLAG=xsan test

test-msan:
	make clean-all
	@if [ ${CXX} != "clang++" ]; then \
		make SAN_FLAG=msan test ; \
	else \
		make test ; \
	fi

test-usan:
	make clean-all
	make SAN_FLAG=usan test

test-tsan:
	make clean-all
	make SAN_FLAG=tsan test

test-memcheck: $(SYSTEM_TEST_EXEC)
	echo "Running test-memcheck"
	@valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--error-exitcode=2 \
		--track-origins=yes \
		--verbose \
		--fair-sched=yes \
		--log-file=$(LOG_DIR)/${SYSTEM_TEST_NAME}_valgrind_log.txt \
		$(SYSTEM_TEST_EXEC) ; \
	FAIL=$$? ; \
	if [ $$FAIL -ne 0 ]; then \
		 echo FAILED test-memcheck ; \
		 exit 2 ; \
	fi ; \

test-helgrind: $(SYSTEM_TEST_EXEC)
	echo "Running test-helgrind"
	@valgrind \
		--tool=helgrind \
		--history-level=full \
		--error-exitcode=2 \
		--fair-sched=yes \
		--verbose \
		--log-file=$(LOG_DIR)/${SYSTEM_TEST_NAME}_helgrind_log.txt \
		$(SYSTEM_TEST_EXEC) ; \
	FAIL=$$? ; \
	if [ $$FAIL -ne 0 ]; then \
		echo FAILED test-memcheck ; \
		exit 2 ; \
	fi ; \

test-sanitizers:  test-xsan test-msan test-tsan

# Make sure to call 'make clean-all' after any sanitizer test as valgrind tools do not mix well with sanitizer instrumented
# code.
test-valgrind: test-memcheck test-helgrind

install:
	install -d $(DESTDIR)$(PREFIX)/$(PROJ_NAME)/include/
	find $(INC_DIR) -type f -exec install -D -m 644 {} "$(DESTDIR)$(PREFIX)/$(PROJ_NAME)/{}" \;

	install -d $(DESTDIR)$(PREFIX)/$(PROJ_NAME)/bin/
	install -m 755 $(MAIN_EXEC) $(DESTDIR)$(PREFIX)/$(PROJ_NAME)/bin/

uninstall:
	rm -rf $(DESTDIR)$(PREFIX)/$(PROJ_NAME)

clean:
	rm -rf $(BUILD_DIR)

clean-all: clean
	rm -rf $(BIN_DIR)