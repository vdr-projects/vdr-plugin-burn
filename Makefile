#
# Makefile for the burn Video Disk Recorder plugin
#
# See the main source file 'burn.c' for copyright information and
# how to reach the author.
#
# $Id:$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.

PLUGIN = burn

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'const char\* plugin::VERSION *=' $(PLUGIN).c | awk '{ print $$5 }' | sed -e 's/[";]//g')

### The directory environment:
#VDRDIR = ../../..

# Use package data if installed...otherwise assume we're under the VDR source directory:
PKGCFG = $(if $(VDRDIR),$(shell pkg-config --variable=$(1) $(VDRDIR)/vdr.pc),$(shell pkg-config --variable=$(1) vdr || pkg-config --variable=$(1) ../../../vdr.pc))
LIBDIR = $(call PKGCFG,libdir)
LOCDIR = $(call PKGCFG,locdir)
PLGCFG = $(call PKGCFG,plgcfg)
#
TMPDIR ?= /tmp

### The compiler options:

export CFLAGS   = $(call PKGCFG,cflags)
export CXXFLAGS = $(call PKGCFG,cxxflags)

### The version number of VDR's plugin API:

APIVERSION = $(call PKGCFG,apiversion)


### Backward compatibility stuff
ifeq ($(LOCDIR),)
### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -O3 -Wall -Woverloaded-virtual -fPIC

### The directory environment:

VDRDIR = ../../..
LIBDIR = ../../lib

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

ifdef DEBUG
	CXXFLAGS = -g -Wall -Woverloaded-virtual -fPIC
else
	DEFINES += -DNDEBUG
endif

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)

LOCDIR = $(VDRDIR)/locale

### Includes and Defines (add further entries here):

DEFINES += -D_GNU_SOURCE

INCLUDES += -I$(VDRDIR)/include

### Run 'install' for target 'all':

all: install

ifeq ($(shell grep 'I18N_DEFAULT_LOCALE' $(VDRDIR)/i18n.h),)
OLD_I18N = 1

i18n install-i18n:

endif
else
### Allow user defined options to overwrite defaults:

-include $(PLGCFG)
endif # compatibility section

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### The name of the shared object file:

SOFILE = libvdr-$(PLUGIN).so

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -I. $(shell gdlib-config --cflags)
LIBS     += $(shell gdlib-config --ldflags --libs) -lgd
DEFINES  += -DPLUGIN_NAME='"$(PLUGIN)"' -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -D_LARGEFILE_SOURCE

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

### compile only with ttxtsub support if core VDR is patched
ifneq ($(strip $(wildcard $(VDRDIR)/vdrttxtsubshooks.h)),)
DEFINES += -DTTXT_SUBTITLES
endif

DEFINES += -DTMPDIR='"$(TMPDIR)"' -DDVDDEV='"$(DVDDEV)"' -DISODIR='"$(ISODIR)"'

### The main target:

all: SUBDIRS $(SOFILE) i18n

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Internationalization (I18N):

ifeq ($(OLD_I18N),)
PODIR     = po
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmo    = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs  = $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c)
	xgettext -C -cTRANSLATORS --no-wrap --no-location  -k -ktr -ktrNOOP --package-name=vdr-$(PLUGIN) --package-version=$(VERSION) --msgid-bugs-address='<see README>' -o $@ `ls $^`

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	install -D -m644 $< $@

.PHONY: i18n
i18n: $(I18Nmo) $(I18Npot)

install-i18n: $(I18Nmsgs)
endif

### Targets:

.PHONY: all dist clean SUBDIRS

#SUBDIRS:
$(SUBLIBS):
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" lib$$dir.a ; \
	done

$(SOFILE): $(OBJS) Makefile $(SUBLIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(OBJS) $(LIBS) -o $@

install-lib: $(SOFILE)
	install -D $^ $(DESTDIR)$(LIBDIR)/$^.$(APIVERSION)

scan-test: $(OBJS) proctools scan-test.o
	$(CXX) $(CXXFLAGS) scan-test.o $(OBJS) -o $@ \
		$(LIBS) -ljpeg -lpthread -ldl -lcap \
		$(shell ls $(VDRDIR)/*.o | grep -v vdr.o) $(VDRDIR)/libsi/libsi.a

gd-test: gdwrapper.o gdtest.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

install: install-lib install-i18n

dist: $(I18Npo) clean
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

### Unit testing

TESTOBJS = t/main.o t/common_functions.o

test: $(TESTOBJS) $(OBJS) proctools
	$(CXX) $(CXXFLAGS) $(TESTOBJS) $(OBJS) -o $@ $(LIBS) \
		-lboost_unit_test_framework -ljpeg -lpthread -ldl -lcap \
		$(shell ls $(VDRDIR)/*.o | grep -v vdr.o) $(VDRDIR)/libsi/libsi.a
	./test

env:
	@echo "Configuration:"
	@echo "  $(PLUGIN) $(VERSION)"
	@echo "  APIVERSION $(APIVERSION)"
	@echo "  CXX $(CXX)  CXXFLAGS $(CXXFLAGS)"
	@echo "  VDRDIR $(VDRDIR)  LIBDIR $(LIBDIR)  LOCDIR $(LOCDIR)"
