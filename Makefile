.POSIX:
/ := ./

BUILD_DIR := $/.build

CC := gcc

CFLAGS += -std=c99 -pedantic
CFLAGS += -iquote $/

ifneq ($(EXPLICIT_FLAGS),1)
CFLAGS += @$/base_warnings
else
CFLAGS += $(shell cat base_flags.txt)
endif

LDFLAGS := -fwhole-program
LDFLAGS += -Wl,--gc-sections

LDLIBS += -lc

SRC := $(shell find $/src -type f -name "*.c")
OBJ := $(SRC:%.c=$(BUILD_DIR)/release/%.o)

ifneq (,$(shell find . -maxdepth 1 -type f -name ".fast"))
MAKEFLAGS += -j
endif

#? (default): build the release binary
.PHONY: _start
_start: all

include $/mk-recipes.mk

release-flags := -O3 -DNDEBUG
#? crash: build the release binary, crash
$(eval $(call mk-recipe-binary, crash, SRC, $(release-flags)))

#? debug: build with debug logs an eponym binary
debug-flags := -O2 -fanalyzer -DCR_DEBUG_MODE=1 -g3
$(eval $(call mk-recipe-binary, debug, SRC, $(debug-flags)))

#? check: build with all warnings and sanitizers an eponym binary
check-flags := $(debug-flags) -fsanitize=address,leak,undefined -Wpadded
$(eval $(call mk-recipe-binary, check, SRC, $(check-flags)))

.PHONY: all
all: $(out-crash)

.PHONY: clean
clean: #? clean: removes object files
	@ $(log) "$(yellow)RM$(reset) OBJS"
	@ $(RM) -r $(_clean)

.PHONY: fclean
fclean: clean #? fclean: remove binaries and object files
	@ $(log) "$(yellow)RM$(reset) $(notdir $(_fclean))"
	@ $(RM) -r $(_fclean)

.PHONY: distclean #? distclean: remove all build artifacts
distclean: fclean
distclean:
	@ $(log) "$(yellow)RM$(reset) $(BUILD_DIR)"
	@ $(RM) -r $(BUILD_DIR)

.PHONY: re
.NOTPARALLEL: re
re: fclean all #? re: rebuild the default target

.PHONY: help
help: #? help: show this help message
	@ grep -P "#[?] " $(MAKEFILE_LIST)          \
      | sed -E 's/.*#\? ([^:]+): (.*)/\1 "\2"/' \
	  | xargs printf "  $(blue)%-12s$(reset): $(cyan)%s$(reset)\n"

#? install: package within the provided dir
.PHONY: install
install: $(out-crash)
	install -Dm755 -t $(PREFIX)/bin $(out-crash)

ifneq ($(shell command -v tput),)
  ifneq ($(shell tput colors),0)

mk-color = \e[$(strip $1)m

reset := $(call mk-color, 00)

red := $(call mk-color, 31)
green := $(call mk-color, 32)
yellow := $(call mk-color, 33)
blue := $(call mk-color, 34)
purple := $(call mk-color, 35)
cyan := $(call mk-color, 36)

  endif
endif

NOW = $(shell date +%s%3N)
STIME := $(call NOW)
TIME_NS = $(shell expr $(call NOW) - $(STIME))
TIME_MS = $(shell expr $(call TIME_NS))

BOXIFY = "[$(blue)$(1)$(reset)] $(2)"

log = printf $(call BOXIFY, %6s , %b\n) "$(call TIME_MS)"
