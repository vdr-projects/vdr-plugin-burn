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
BINDIR = $(call PKGCFG,bindir)
CFGDIR = $(call PKGCFG,configdir)/plugins/$(PLUGIN)
RESDIR = $(call PKGCFG,resdir)/plugins/$(PLUGIN)
PLGCFG = $(call PKGCFG,plgcfg)
#
BURN_TMPDIR ?= /tmp

### The compiler options:

export CFLAGS   = $(call PKGCFG,cflags)
export CXXFLAGS = $(call PKGCFG,cxxflags)

### The version number of VDR's plugin API:

APIVERSION = $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:

-include $(PLGCFG)

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


ifndef BURN_TMPDIR
BURN_TMPDIR=/tmp
endif

ifndef BURN_DVDDEV
BURN_DVDDEV=/dev/dvdrw
endif

ifndef BURN_ISODIR
BURN_ISODIR=/pub/export
endif

### compile only with ttxtsub support if core VDR is patched
ifneq ($(strip $(wildcard $(VDRDIR)/vdrttxtsubshooks.h)),)
DEFINES += -DTTXT_SUBTITLES
endif

DEFINES += -DBURN_TMPDIR='"$(BURN_TMPDIR)"' -DBURN_DVDDEV='"$(BURN_DVDDEV)"' -DBURN_ISODIR='"$(BURN_ISODIR)"'

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

install-sh:
	install -d $(DESTDIR)$(BINDIR)
	cp -an scripts/* $(DESTDIR)$(BINDIR)

install-conf:
	install -d $(DESTDIR)$(CFGDIR)
	cp -an config/* $(DESTDIR)$(CFGDIR)

install-res:
	install -d $(DESTDIR)$(RESDIR)
	cp -an resource/* $(DESTDIR)$(RESDIR)

scan-test: $(OBJS) proctools scan-test.o
	$(CXX) $(CXXFLAGS) scan-test.o $(OBJS) -o $@ \
		$(LIBS) -ljpeg -lpthread -ldl -lcap \
		$(shell ls $(VDRDIR)/*.o | grep -v vdr.o) $(VDRDIR)/libsi/libsi.a

gd-test: gdwrapper.o gdtest.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

install: install-lib install-sh install-res install-conf install-i18n

dist: $(I18Npo) clean
	@-rm -rf $(BURN_TMPDIR)/vdrdist/$(ARCHIVE)
	@mkdir -p $(BURN_TMPDIR)/vdrdist/$(ARCHIVE)
	@cp -a * $(BURN_TMPDIR)/vdrdist/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(BURN_TMPDIR)/vdrdist $(ARCHIVE)
	@-rm -rf $(BURN_TMPDIR)/vdrdist
	@echo Distribution package created as $(PACKAGE).tgz


clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f *.o genindex/*.o $(DEPFILE) *.so *.tgz core* *~ \
		scan-test test scripts/*~
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean ; \
	done

env:
	@echo "Configuration:"
	@echo "  $(PLUGIN) $(VERSION)"
	@echo "  APIVERSION $(APIVERSION)"
	@echo "  CXX $(CXX)  CXXFLAGS $(CXXFLAGS)"
	@echo "  VDRDIR $(VDRDIR)  LIBDIR $(LIBDIR)  LOCDIR $(LOCDIR)"
