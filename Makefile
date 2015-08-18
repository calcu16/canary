# Programs
# Compiling C code
SHELL       = /bin/sh
CC          = clang
CFLAGS      = -O -g -Wall -pedantic -Wno-variadic-macros -Wno-format -Wno-overlength-strings -Werror
INCLUDE     = -Isrc
LDFLAGS     =

SRC_DIR     = src
BLD_DIR     = build
DEP_DIR     = $(BLD_DIR)/dep
OBJ_DIR     = $(BLD_DIR)/obj
BIN_DIR	    = $(BLD_DIR)/bin

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

SRC         = src/$*.c
NAME        = $*

# Compile LaTeX
TEX := python bin/pylatex

PAPERS	:= $(patsubst tex/%,%,$(wildcard tex/*))
PDFS    := $(PAPERS:%=build/docs/%.pdf)
BIB     := bib/main.bib

PAPER_SRC  = tex/$(basename $(@F))
PAPER_SRCS = $(wildcard $(SRC)/*)

all: docs targets

clean:
	rm -rf build

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
	$(CC) -o $@ -c $(INCLUDE) $(CFLAGS) $<

$(BIN_DIR)/%:
	@mkdir -p $(@D)
	$(CC) -o $@ $(LDFLAGS) $^



# Building tex files
docs: $(PDFS)

$(PDFS): $$(PAPER_SRCS) $(BIB)
	@mkdir -p $(@D)
	$(TEX) $(PAPER_SRC)/paper.tex $@ bib/main.bib

