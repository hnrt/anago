# Copyright (C) 2012-2017 Hideaki Narita

PROJECTROOT=../../

BINROOT=$(PROJECTROOT)support/bin
BINDIR=$(BINROOT)/$(PLATFORM)/$(CONFIGURATION)
OBJROOT=obj
OBJDIR=$(OBJROOT)/$(PLATFORM)/$(CONFIGURATION)

PLATFORM=linux
CONFIGURATION=debug

######################################################################

COMPILE=$(CC) $(CFLAGS) $(CPPFLAGS) -c
LINK=$(LD) $(LDFLAGS)
ARCHIVE=$(AR) $(ARFLAGS)

CC=g++
CFLAGS=$(STDCFLAGS) $(USRCFLAGS) $(EXTCFLAGS)
CPPFLAGS=$(STDCPPFLAGS) $(USRCPPFLAGS) $(EXTCPPFLAGS)
LD=g++
LDFLAGS=$(STDLDFLAGS) $(USRLDFLAGS) $(EXTLDFLAGS)
AR=ar
ARFLAGS=$(STDARFLAGS) $(USRARFLAGS) $(EXTARFLAGS)

STDCFLAGS=-Wall -Werror
STDCPPFLAGS=-DLINUX -D_GNU_SOURCE
STDLDFLAGS=
STDARFLAGS=ruv

ifeq ($(CONFIGURATION), release)
#USRCFLAGS=-O3 -fno-strict-aliasing
USRCFLAGS=-O3 $(GTKMMCFLAGS)
USRCPPFLAGS=-I./Common $(HNRTINC) -I/usr/local/lib
else
USRCFLAGS=-g $(GTKMMCFLAGS)
USRCPPFLAGS=-D_DEBUG -I./Common $(HNRTINC) -I/usr/local/lib
endif
USRLDFLAGS=-Wl,-rpath,/usr/local/lib
USRARFLAGS=
USRLIBS=$(GTKMMLIBS) -luuid

######################################################################

RM=rm -f
RMALL=rm -fr
MKDIR=mkdir
MKDIRS=mkdir -p

######################################################################

$(OBJDIR)/%.o: %.cc
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(COMPILE) -o $@ $<

######################################################################

all::

clean distclean::
	$(RM) *~

clean::
	$(RMALL) $(OBJDIR)

distclean::
	$(RMALL) $(OBJROOT)

distclean::
	$(RMALL) $(BINROOT)

both:: release debug

release debug::
	$(MAKE) all CONFIGURATION=$@

release-clean::
	$(MAKE) clean CONFIGURATION=release

debug-clean::
	$(MAKE) clean CONFIGURATION=debug

######################################################################

SUBDIRS=\
App \
Common

clean distclean::
	for d in $(SUBDIRS); do $(RM) $$d/*~; done

######################################################################

PROJ1=$(BINDIR)/potmerge
OBJS1=\
$(OBJDIR)/App/potmerge.o \
$(OBJDIR)/Common/MsgItem.o \
$(OBJDIR)/Common/PoFile.o
LIBS1=$(STDLIBS) $(USRLIBS) $(EXTLIBS) $(HNRTLIB)

$(PROJ1): $(OBJS1)
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(LINK) -o $@ $(OBJS1) $(LIBS1)

all:: $(PROJ1)

######################################################################

PROJ2=$(BINDIR)/poupdate
OBJS2=\
$(OBJDIR)/App/poupdate.o \
$(OBJDIR)/Common/MsgItem.o \
$(OBJDIR)/Common/PoFile.o
LIBS2=$(STDLIBS) $(USRLIBS) $(EXTLIBS) $(HNRTLIB)

$(PROJ2): $(OBJS2)
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(LINK) -o $@ $(OBJS2) $(LIBS2)

all:: $(PROJ2)

######################################################################

PROJ3=$(BINDIR)/pocheck
OBJS3=\
$(OBJDIR)/App/pocheck.o \
$(OBJDIR)/Common/MsgItem.o \
$(OBJDIR)/Common/PoFile.o
LIBS3=$(STDLIBS) $(USRLIBS) $(EXTLIBS) $(HNRTLIB)

$(PROJ3): $(OBJS3)
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(LINK) -o $@ $(OBJS3) $(LIBS3)

all:: $(PROJ3)

######################################################################
