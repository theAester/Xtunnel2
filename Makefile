CC=gcc
INCLUDEPATH=include/
SRCPATH=src/
BUILDPATH=build/
PROGNAME=$(BUILDPATH)Xtun
CCARGS=-g
CCARGS+=-I$(INCLUDEPATH)

BARGS=

LARGS=-lpthread

CFILES=$(wildcard $(SRCPATH)*.c)
OFILES=$(patsubst %.c, %.o,$(CFILES))

compile: $(PROGNAME)

$(PROGNAME): $(BUILDPATH) $(OFILES)
	$(CC) $(CCARGS) $(BARGS) -o $(PROGNAME) $(OFILES) $(LARGS)

%.o: %.c
	$(CC) -c $(CCARGS) -o $@ $< $(LARGS)

$(BUILDPATH):
	mkdir build

clean:
	rm $(OFILES)
	rm $(PROGNAME)

tidy:
	rm $(OFILES)
