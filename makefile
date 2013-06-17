
CC=g++
INCLUDES=-I../avtk/ -I/usr/include/cairomm-1.0 -I/usr/lib/cairomm-1.0/include -I/usr/include/sigc++-2.0 -I/usr/lib/sigc++-2.0/include -I/usr/include/ntk -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng15
CFLAGS=-g -c -Wall
LDFLAGS=-lntk -lcairomm-1.0 -lcairo -lsndfile -z nodelete

UISOURCES=gui/ui_helpers.cxx gui/fabla.cxx gui/fabla_ui.c
UIOBJECTS=$(UISOURCES:.cpp=.o)
UI=fabla.lv2/fabla_ui.so

SASOURCES=gui/fabla.cxx gui/ui_test.cxx
SAOBJECTS=$(SASOURCES:.cpp=.o)
SA=uiTest

DSPSOURCES=dsp/fabla.c
DSPOBJECTS=$(DSPSOURCES:.cpp=.o)
DSP=fabla.lv2/fabla.so

all: folder ui dsp

genUI:
	cd gui; ntk-fluid -c fabla.fl

folder:
	mkdir -p fabla.lv2/

install:
	cp dsp/fabla.ttl fabla.lv2/
	cp -r fabla.lv2 ~/.lv2/

ui: $(UISOURCES) $(UI)
dsp: $(DSPSOURCES) $(DSP)

# for testing only: builds a standalone NTK app
sa: $(SASOURCES) $(SA)

$(DSP): $(DSPOBJECTS)
	$(CC) $(INCLUDES) -fPIC -shared $(LDFLAGS) $(DSPOBJECTS) -o $@

$(UI): $(UIOBJECTS)
	$(CC) $(INCLUDES) -fPIC -shared $(LDFLAGS) $(UIOBJECTS)  -o $@

$(SA): $(SAOBJECTS)
	$(CC) -g $(INCLUDES) $(LDFLAGS) $(SAOBJECTS) -o $@

.cpp.o:
	$(CC) -g $(CFLAGS) $< -o $@



clean:
	rm -f uiTest
	rm -f fabla.lv2/fabla.so
	rm -f fabla.lv2/fabla_ui.so
