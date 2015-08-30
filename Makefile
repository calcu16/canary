# Programs
# Compiling C code
SHELL       = /bin/sh
CC          = clang
CFLAGS     := -Wall -pedantic -Wno-variadic-macros -Wno-format -Wno-overlength-strings -Werror
CRFLAGS    := -Ofast -DNDEBUG
CDFLAGS    := -g
INCLUDE     = -Isrc
LDFLAGS     =

SRC_DIR     = src
BLD_DIR     = build
DEP_DIR     = $(BLD_DIR)/dep
OBJ_DIR     = $(BLD_DIR)/obj
BIN_DIR     = $(BLD_DIR)/bin
TST_DIR     = $(BLD_DIR)/tst

OBJDEP      = $(NAME:%=$(DEP_DIR)/%.od)
BINDEP      = $(NAME:%=$(DEP_DIR)/%.d)
OBJECT      = $(NAME:%=$(OBJ_DIR)/%.o)
TARGET      = $(NAME:%=$(BIN_DIR)/%)

NAME        = $(BINSRCS:src/%.c=%)
BINSRCS    := $(shell bin/find_bin_srcs.sh $(SRC_DIR))
TARGETS    := $(TARGET)
BINDEPS    := $(BINDEP)

NAME        = $(OBJSRCS:src/%.c=%)
OBJSRCS    := $(shell bin/find_srcs.sh $(SRC_DIR))
OBJDEPS    := $(OBJDEP)
OBJECTS    := $(OBJECT)

DEPENDS    := $(BINDEPS) $(OBJDEPS)

# Debug builds
DBG_DIR     = $(BLD_DIR)/dbg
DDEP_DIR    = $(DBG_DIR)/dep
DOBJ_DIR    = $(DBG_DIR)/obj
DBIN_DIR    = $(DBG_DIR)/bin

DOBJDEP      = $(NAME:%=$(DDEP_DIR)/%.od)
DBINDEP      = $(NAME:%=$(DDEP_DIR)/%.d)
DOBJECT      = $(NAME:%=$(DOBJ_DIR)/%.o)
DTARGET      = $(NAME:%=$(DBIN_DIR)/%)

NAME        = $(DBINSRCS:src/%.c=%)
DBINSRCS   := $(shell bin/find_bin_srcs.sh $(SRC_DIR))
DTARGETS   := $(DTARGET)
DBINDEPS   := $(DBINDEP)

NAME        = $(DOBJSRCS:src/%.c=%)
DOBJSRCS   := $(shell bin/find_srcs.sh $(SRC_DIR))
DOBJDEPS   := $(DOBJDEP)
DOBJECTS   := $(DOBJECT)

DDEPENDS    := $(DBINDEPS) $(DOBJDEPS)

SRC         = src/$*.c
NAME        = $*

# Testing binaries
TESTS      := $(wildcard tst/*)
RESULTS    := $(TESTS:%=$(BLD_DIR)/%.result)
PASSES     := $(TESTS:%=$(BLD_DIR)/%.pass)
FAILS      := $(TESTS:%=$(BLD_DIR)/%.fail)
ARGVS      := $(wildcard $(TESTS:%=%/*.argv))
DIFFS      := $(ARGVS:%.argv=$(BLD_DIR)/%.diff)
OUTS       := $(ARGVS:%.argv=$(BLD_DIR)/%.out)

TEST        = $(basename $(@F))
TPROG       = $(notdir $(@D))
TNAME       = $(basename $(@F))
PASS        = $(TEST:%=$(TST_DIR)/%.pass)
FAIL        = $(TEST:%=$(TST_DIR)/%.fail)
DIFF        = $(filter $(TST_DIR)/$(TEST)/%,$(DIFFS))
OUT         = $(@:%.diff=%.out)
GOLD_OUT    = $(@:build/%.diff=%.out)
BIN         = build/bin/$(TPROG)
ARGV        = tst/$(TPROG)/$(TNAME).argv

# Compile LaTeX
TEX := python bin/pylatex.py

PAPERS	:= $(patsubst tex/%,%,$(wildcard tex/*))
PDFS    := $(PAPERS:%=build/docs/%.pdf)
BIB     := bib/main.bib

PAPER_SRC  = tex/$(basename $(@F))
PAPER_SRCS = $(wildcard $(PAPER_SRC)/*)

all: docs targets

clean:
	rm -rf build

debug: dtargets

test: $(RESULTS)
	@cat $(RESULTS)

.PHONY: all clean test
.SECONDEXPANSION:

# Building C programs
targets: $(TARGETS)

$(DEP_DIR)/%.od: $$(SRC)
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDE) $(SRC) >/dev/null || (echo $(CC) -MM $(INCLUDE) $(SRC); false)
	$(CC) -MM $(INCLUDE) $(SRC) |\
	sed -e "s,^[^:]*: $(SRC_DIR)/\([^ ]*\).c,$(DEP_DIR)/\1.od $(OBJ_DIR)/\1.o: $(SRC_DIR)/\1.c," >$@

$(DEP_DIR)/%.d: $$(OBJDEP)
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDE) $(OBJSRCS) >/dev/null || (echo $(CC) -MM $(INCLUDE) $(OBJSRCS); false)
	$(CC) -MM $(INCLUDE) $(OBJSRCS) |\
        python bin/multiline.py |\
	sed "s,[ ][ ]*, ,g" |\
	python bin/make_bin_deps.py $(NAME) $@ $(SRC_DIR) $(OBJ_DIR) $(BIN_DIR) >$@

-include $(DEPENDS)

$(OBJ_DIR)/%.o:
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(INCLUDE) $(CFLAGS) $(CRFLAGS) $<

$(BIN_DIR)/%:
	@mkdir -p $(@D)
	$(CC) -o $@ $(LDFLAGS) $^

# Building debug versions of C programs
dtargets: $(DTARGETS)

$(DDEP_DIR)/%.od: $$(SRC)
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDE) $(SRC) >/dev/null || (echo $(CC) -MM $(INCLUDE) $(SRC); false)
	$(CC) -MM $(INCLUDE) $(SRC) |\
	sed -e "s,^[^:]*: $(SRC_DIR)/\([^ ]*\).c,$(DDEP_DIR)/\1.od $(DOBJ_DIR)/\1.o: $(SRC_DIR)/\1.c," >$@

$(DDEP_DIR)/%.d: $$(DOBJDEP)
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDE) $(DOBJSRCS) >/dev/null || (echo $(CC) -MM $(INCLUDE) $(DOBJSRCS); false)
	$(CC) -MM $(INCLUDE) $(DOBJSRCS) |\
        python bin/multiline.py |\
	sed "s,[ ][ ]*, ,g" |\
	python bin/make_bin_deps.py $(NAME) $@ $(SRC_DIR) $(DOBJ_DIR) $(DBIN_DIR) >$@

-include $(DDEPENDS)

$(DOBJ_DIR)/%.o:
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(INCLUDE) $(CDFLAGS) $(CFLAGS) $<

$(DBIN_DIR)/%:
	@mkdir -p $(@D)
	$(CC) -o $@ $(LDFLAGS) -g $^


# Build tests
$(RESULTS): $$(PASS) $$(FAIL)
	@mkdir -p $(@D)
	wc -l $(PASS) $(FAIL) | sed 's/^[ ]*\([0-9]*\)[ ].*/\1/' | xargs | awk '{printf "%s: %d passed and %d failed\n", "$(TEST)", $$1, $$2}' >$@

$(PASSES): $$(DIFF)
	@mkdir -p $(@D)
	wc -l $(DIFF) | sed -n 's/^[ ]*0[ ].*\/\(.*\).diff/\1/p' >$@

$(FAILS): $$(DIFF)
	@mkdir -p $(@D)
	wc -l $(DIFF) | sed -n 's/^[ ]*[1-9][0-9]*[ ].*\/\(.*\).diff/\1/p' >$@

$(DIFFS): $$(OUT) $$(GOLD_OUT)
	@mkdir -p $(@D)
	bin/diff.py $(OUT) $(GOLD_OUT) >$@

$(OUTS): $$(BIN) $$(ARGV)
	@mkdir -p $(@D)
	cat $(ARGV) | bin/lxargs.py $(BIN) >$@

# Building tex files
docs: $(PDFS)

$(PDFS): $$(PAPER_SRCS) $(BIB)
	@mkdir -p $(@D)
	$(TEX) $(PAPER_SRC)/paper.tex $@ bib/main.bib

