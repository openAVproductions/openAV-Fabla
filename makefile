
CC=g++
CFLAGS=-g -Wall

INCLUDES=$(shell pkg-config --cflags sndfile cairomm-1.0 ntk ntk_images)
LDFLAGS=$(shell pkg-config --libs sndfile cairomm-1.0 ntk ntk_images) -fPIC -shared -Wl,-z,nodelete

UISOURCES=gui/ui_helpers.cxx gui/fabla.cxx gui/fabla_ui.c
UIOBJECTS=$(UISOURCES:.cpp=.o)
UI=fabla.lv2/fabla_ui.so

SASOURCES=gui/fabla.cxx gui/ui_helpers.cxx gui/ui_test.cxx
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
	cp -r presets/*.lv2 ~/.lv2/

ui: $(UISOURCES) $(UI)
dsp: $(DSPSOURCES) $(DSP)

# for testing only: builds a standalone NTK app
sa: $(SASOURCES) $(SA)

$(DSP): $(DSPOBJECTS)
	$(CC) $(DSPOBJECTS) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -o $@

$(UI): $(UIOBJECTS)
	$(CC) $(UIOBJECTS)  $(INCLUDES) $(CFLAGS) $(LDFLAGS) -o $@

$(SA): $(SAOBJECTS)
	$(CC) $(SAOBJECTS) $(INCLUDES) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $< -g $(CFLAGS) -c -o $@



clean:
	rm -f uiTest
	rm -f fabla.lv2/fabla.so
	rm -f fabla.lv2/fabla_ui.so
