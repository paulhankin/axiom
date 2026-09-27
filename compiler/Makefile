# Phalange compiler (C11 + LLVM C API)
#
#   make                 release build -> build/phalange
#   make test            run the test suite (needs node)
#   make asan            sanitizer build -> build-asan/phalange
LLVM_CONFIG ?= llvm-config
CC ?= cc
BUILD ?= build

CFLAGS ?= -O2 -g
LDFLAGS ?=
REQ_CFLAGS := -std=c11 -D_DEFAULT_SOURCE -D_GNU_SOURCE -Wall -Wextra -Wno-unused-parameter \
              -I$(shell $(LLVM_CONFIG) --includedir)
REQ_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags)
LDLIBS := $(shell $(LLVM_CONFIG) --libs) -lm

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))

$(BUILD)/phalange: $(OBJS)
	$(CC) $(LDFLAGS) $(REQ_LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BUILD)/%.o: src/%.c src/*.h | $(BUILD)
	$(CC) $(REQ_CFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(BUILD)/phalange
	node ../tests/run.mjs

asan:
	$(MAKE) BUILD=build-asan CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" \
	        LDFLAGS="-fsanitize=address,undefined"

clean:
	rm -rf build build-asan

.PHONY: test asan clean
