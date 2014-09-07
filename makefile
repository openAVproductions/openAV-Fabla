
CC=g++
CFLAGS=-g -Wall

ifeq ($(shell pkg-config --atleast-version=1.8.0 lv2 || echo no), no)
  $(error "LV2 >= 1.8.0 was not found, please update the lv2 package")
endif

INCLUDES=$(shell pkg-config --cflags ntk lv2 cairomm-1.0 sndfile ntk_images)
LDFLAGS=$(shell pkg-config --libs ntk lv2 cairomm-1.0 sndfile ntk_images) -fPIC -shared -Wl,-z,nodelete

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
	mkdir -p ~/.lv2
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
	$(CC)  $(CFLAGS)  $(INCLUDES) $(UIOBJECTS) $(LDFLAGS) -o $@

$(SA): $(SAOBJECTS)
	$(CC) $(SAOBJECTS) $(INCLUDES) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $< -g $(CFLAGS) -c -o $@



clean:
	rm -f uiTest
	rm -f fabla.lv2/fabla.so
	rm -f fabla.lv2/fabla_ui.so
