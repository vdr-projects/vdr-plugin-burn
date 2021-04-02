#
# Makefile for a Video Disk Recorder plugin
#
# $Id: Makefile,v 1.23 2006/10/01 21:26:44 lordjaxom Exp $

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
#
PLUGIN = burn

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'const char\* plugin::VERSION *=' $(PLUGIN).c | awk '{ print $$5 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -O3 -Wall -Woverloaded-virtual -fPIC

### The directory environment:

VDRDIR = ../../..
LIBDIR = ../../lib
TMPDIR = /tmp

### Make sure that necessary options are included:

-include $(VDRDIR)/Make.global

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

ifdef DEBUG
	CXXFLAGS = -g -Wall -Woverloaded-virtual -fPIC
else
	DEFINES += -DNDEBUG
endif

### The version number of VDR (taken from VDR's "config.h"):

VDRVERSION = $(shell sed -ne '/define VDRVERSION/ { s/^.*"\(.*\)".*$$/\1/; p }' $(VDRDIR)/config.h)
APIVERSION = $(shell sed -ne '/define APIVERSION/ { s/^.*"\(.*\)".*$$/\1/; p }' $(VDRDIR)/config.h)
ifeq ($(APIVERSION),)
	APIVERSION = $(VDRVERSION)
endif
VDRVERSNUM = $(shell sed -ne '/define VDRVERSNUM/ s/^.[a-zA-Z ]*\([0-9]*\) .*$$/\1/p' $(VDRDIR)/config.h)
APIVERSNUM = $(shell sed -ne '/define APIVERSNUM/ s/^.[a-zA-Z ]*\([0-9]*\) .*$$/\1/p' $(VDRDIR)/config.h)
ifeq ($(strip $(APIVERSNUM)),)
	APIVERSNUM = $(VDRVERSNUM)
endif

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -I. $(shell gdlib-config --cflags)
LIBS     += $(shell gdlib-config --ldflags --libs) -lgd
DEFINES  += -DPLUGIN_NAME='"$(PLUGIN)"' -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -D_GNU_SOURCE -D_LARGEFILE_SOURCE

### The object files (add further files here):

OBJS = burn.o chain-vdr.o chain-dvd.o jobs.o logger-vdr.o skins.o \
	   manager.o menuburn.o menubase.o \
	   etsi-const.o tracks.o scanner.o gdwrapper.o iconvwrapper.o \
	   menuitems.o setup.o common.o config.o render.o \
	   genindex/pes.o

ifdef ENABLE_DMH_ARCHIVE
DEFINES += -DENABLE_DMH_ARCHIVE
OBJS += chain-archive.o
endif

SUBDIRS = proctools # tinyxml
SUBLIBS=$(shell for i in $(SUBDIRS); do echo $$i/lib$$i.a; done)

LIBS += $(SUBLIBS)


ifndef TMPDIR
TMPDIR=/tmp
endif

ifndef DVDDEV
DVDDEV=/dev/dvdrw
endif

ifndef ISODIR
ISODIR=/pub/export
endif

# compile only with ttxtsub support if core VDR is patched
ifneq ($(strip $(wildcard $(VDRDIR)/vdrttxtsubshooks.h)),)
DEFINES += -DTTXT_SUBTITLES
endif


DEFINES += -DTMPDIR='"$(TMPDIR)"' -DDVDDEV='"$(DVDDEV)"' -DISODIR='"$(ISODIR)"'

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmsgs  = $(addprefix $(LOCALEDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<<vdr@smue.org>>' -o $@ $^

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: i18n
i18n: $(I18Nmsgs)

### Targets:

.PHONY: all dist clean SUBDIRS

all: SUBDIRS libvdr-$(PLUGIN).so i18n

SUBDIRS:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" lib$$dir.a ; \
	done

libvdr-$(PLUGIN).so: $(OBJS) Makefile $(SUBLIBS)
	$(CXX) $(CXXFLAGS) -shared $(OBJS) $(LIBS) -o $@
	@cp $@ $(LIBDIR)/$@.$(APIVERSION)

scan-test: $(OBJS) proctools scan-test.o
	$(CXX) $(CXXFLAGS) scan-test.o $(OBJS) -o $@ \
		$(LIBS) -ljpeg -lpthread -ldl -lcap \
		$(shell ls $(VDRDIR)/*.o | grep -v vdr.o) $(VDRDIR)/libsi/libsi.a

gd-test: gdwrapper.o gdtest.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

dist: clean
	@-rm -rf $(TMPDIR)/vdrdist/$(ARCHIVE)
	@mkdir -p $(TMPDIR)/vdrdist/$(ARCHIVE)
	@cp -a * $(TMPDIR)/vdrdist/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR)/vdrdist $(ARCHIVE)
	@-rm -rf $(TMPDIR)/vdrdist
	@echo Distribution package created as $(PACKAGE).tgz


clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f *.o genindex/*.o $(DEPFILE) *.so *.tgz core* *~ \
		scan-test test t/*.o
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean ; \
	done

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Unit testing

TESTOBJS = t/main.o t/common_functions.o

test: $(TESTOBJS) $(OBJS) proctools
	$(CXX) $(CXXFLAGS) $(TESTOBJS) $(OBJS) -o $@ $(LIBS) \
		-lboost_unit_test_framework -ljpeg -lpthread -ldl -lcap \
		$(shell ls $(VDRDIR)/*.o | grep -v vdr.o) $(VDRDIR)/libsi/libsi.a
	./test

