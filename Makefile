# Programs
TEX := python bin/pylatex


PAPERS	:= $(patsubst tex/%,%,$(wildcard tex/*))
PDFS    := $(PAPERS:%=build/docs/%.pdf)
BIB     := bib/main.bib

PAPER_SRC  = tex/$(basename $(@F))
PAPER_SRCS = $(wildcard $(SRC)/*)

all: docs

clean:
	rm -rf build

.SECONDEXPANSION:

# Building tex files
docs: $(PDFS)

$(PDFS): $$(PAPER_SRCS) $(BIB)
	@mkdir -p $(@D)
	$(TEX) $(PAPER_SRC)/paper.tex $@ bib/main.bib

