# Axiom compiler (C11 + LLVM C API)
#
#   make                 release build -> build/axiom
#   make test            run the test suite (needs node)
#   make asan            sanitizer build -> build-asan/axiom
# LLVM 18's llvm-config: as Debian and Ubuntu name it, from Homebrew's llvm@18, or
# whichever is on the PATH (set LLVM_CONFIG to choose)
LLVM_CONFIG ?= $(firstword $(foreach c,llvm-config-18 /opt/homebrew/opt/llvm@18/bin/llvm-config \
                 /usr/local/opt/llvm@18/bin/llvm-config llvm-config,$(shell command -v $(c) 2>/dev/null)))
ifeq ($(LLVM_CONFIG),)
$(error no llvm-config: install LLVM 18 (apt install llvm-18-dev lld, or brew install llvm@18), or set LLVM_CONFIG)
endif
ifneq ($(firstword $(subst ., ,$(shell $(LLVM_CONFIG) --version))),18)
$(error $(LLVM_CONFIG) is LLVM $(shell $(LLVM_CONFIG) --version), not 18: install LLVM 18 (apt install llvm-18-dev lld, or brew install llvm@18), or set LLVM_CONFIG)
endif
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

$(BUILD)/axiom: $(OBJS)
	$(CC) $(LDFLAGS) $(REQ_LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BUILD)/%.o: src/%.c src/*.h | $(BUILD)
	$(CC) $(REQ_CFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(BUILD)/axiom
	node ../../../tests/run.mjs

asan:
	$(MAKE) BUILD=build-asan CFLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" \
	        LDFLAGS="-fsanitize=address,undefined"

clean:
	rm -rf build build-asan

.PHONY: test asan clean
