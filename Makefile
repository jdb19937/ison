# Makefile — aedificatio libison.a
#
# Usus:
#   make            — aedifica libison.a
#   make purga     — dele omnia aedificata
#   make proba      — aedifica et curre probationes (si adsunt)

CC      ?= cc
AR      ?= ar
CFLAGS  ?= -Wall -Wextra -pedantic -std=c99 -O2
ARFLAGS  = rcs

FONTES  = ison.c
OBJECTA = $(FONTES:.c=.o)
BIBLIOTHECA = libison.a

.PHONY: omnia purga proba

omnia: $(BIBLIOTHECA)

$(BIBLIOTHECA): $(OBJECTA)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c ison.h
	$(CC) $(CFLAGS) -c -o $@ $<

purga:
	rm -f $(OBJECTA) $(BIBLIOTHECA)

proba: $(BIBLIOTHECA)
	@echo "libison.a aedificata est. nulla probatio adhuc."
