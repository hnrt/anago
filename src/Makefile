# Copyright (C) 2012-2017 Hideaki Narita

PROJNAME=anago
TARGETEXE=$(PROJNAME)
PACKAGENAME=$(PROJNAME)
DEFAULTDOMAIN=$(PROJNAME)

PROJECTROOT=../

BINROOT=$(PROJECTROOT)bin
BINDIR=$(BINROOT)/$(PLATFORM)/$(CONFIGURATION)
OBJROOT=obj
OBJDIR=$(OBJROOT)/$(PLATFORM)/$(CONFIGURATION)

PLATFORM=linux
CONFIGURATION=debug

######################################################################

COMPILE=$(CC) $(CFLAGS) $(CPPFLAGS) -c
LINK=$(LD) $(LDFLAGS)

CC=g++
CFLAGS=$(STDCFLAGS) $(USRCFLAGS) $(EXTCFLAGS)
CPPFLAGS=$(STDCPPFLAGS) $(USRCPPFLAGS) $(EXTCPPFLAGS)
LD=g++
LDFLAGS=$(STDLDFLAGS) $(USRLDFLAGS) $(EXTLDFLAGS)

STDCFLAGS=-Wall -Werror
STDCPPFLAGS=-DLINUX -D_GNU_SOURCE
STDLDFLAGS=
STDLIBS=

ifeq ($(CONFIGURATION), release)
#USRCFLAGS=-O3 -fno-strict-aliasing
USRCFLAGS=-O3 $(GTKMMCFLAGS) $(XSCFLAGS) $(CURLCFLAGS) $(XML2CFLAGS)
USRCPPFLAGS=-I.
else
USRCFLAGS=-g $(GTKMMCFLAGS) $(XSCFLAGS) $(CURLCFLAGS) $(XML2CFLAGS)
USRCPPFLAGS=-D_DEBUG -I.
endif
USRLDFLAGS=-Wl,-rpath,/usr/local/lib
USRLIBS=$(GTKMMLIBS) $(XSLIBS) $(CURLLIBS) $(XML2LIBS) -luuid

GTKMMCFLAGS=`pkg-config --cflags gtkmm-2.4`
GTKMMLIBS=`pkg-config --libs gtkmm-2.4`

CURLCFLAGS=`curl-config --cflags`
CURLLIBS=`curl-config --libs`

XML2CFLAGS=`xml2-config --cflags`
XML2LIBS=`xml2-config --libs`

XSCFLAGS=-I /usr/local/include
XSLIBS=-L /usr/local/lib -lxenserver

######################################################################

RM=rm -f
RMALL=rm -fr
MKDIR=mkdir
MKDIRS=mkdir -p

######################################################################

$(OBJDIR)/%.o: %.cc
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(COMPILE) -o $@ $<

$(OBJDIR)/%.pot: %.cc
	@test -d $(@D) || $(MKDIRS) $(@D)
	xgettext --package-name $(PACKAGENAME) --default-domain $(DEFAULTDOMAIN) --output $@ $<

######################################################################

all::

clean distclean::
	$(RM) *~

clean::
	$(RMALL) $(BINDIR)

distclean::
	$(RMALL) $(BINROOT)

clean::
	$(RMALL) $(OBJDIR)

distclean::
	$(RMALL) $(OBJROOT)

release debug::
	$(MAKE) all CONFIGURATION=$@

release-clean::
	$(MAKE) clean CONFIGURATION=release

debug-clean::
	$(MAKE) clean CONFIGURATION=debug

######################################################################

SUBDIRS=\
App \
Base \
Controller \
Env \
Logger \
Model \
View

clean distclean::
	for d in $(SUBDIRS); do $(RM) $$d/*~ ; done

######################################################################

PROJ1=$(BINDIR)/$(TARGETEXE)
LIBS1=$(STDLIBS) $(USRLIBS)
OBJS1=$(OBJDIR)/App/Main.o \
$(OBJDIR)/Base/InterlockedCompareExchange.o \
$(OBJDIR)/Base/InterlockedExchange.o \
$(OBJDIR)/Base/InterlockedExchangeAdd.o \
$(OBJDIR)/Controller/Background.o \
$(OBJDIR)/Controller/Controller.o \
$(OBJDIR)/Controller/ControllerImpl.o \
$(OBJDIR)/Env/Env.o \
$(OBJDIR)/Env/Locale.o \
$(OBJDIR)/Env/LocaleImpl.o \
$(OBJDIR)/Env/Process.o \
$(OBJDIR)/Env/ProcessImpl.o \
$(OBJDIR)/Logger/LogLevel.o \
$(OBJDIR)/Logger/Logger.o \
$(OBJDIR)/Logger/LoggerImpl.o \
$(OBJDIR)/Model/Model.o \
$(OBJDIR)/Model/ModelImpl.o \
$(OBJDIR)/View/MainWindow.o \
$(OBJDIR)/View/View.o \
$(OBJDIR)/View/ViewImpl.o

######################################################################

$(PROJ1): $(OBJS1)
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(LINK) -o $@ $(OBJS1) $(LIBS1)
ifeq ($(CONFIGURATION), release)
	strip $(PROJ1)
endif

all:: $(PROJ1)

clean::
	$(RM) $(PROJ1)

######################################################################

$(OBJDIR)/App/Main.o: App/Main.cc App/Constants.h Controller/Controller.h Env/Env.h Env/Locale.h Logger/LogLevel.h Logger/Logger.h Model/Model.h View/View.h 
$(OBJDIR)/Controller/Background.o: Controller/Background.cc Controller/Background.h Controller/Controller.h 
$(OBJDIR)/Controller/Controller.o: Controller/Controller.cc Controller/Controller.h Controller/ControllerImpl.h Logger/LogLevel.h Logger/Logger.h 
$(OBJDIR)/Controller/ControllerImpl.o: Controller/ControllerImpl.cc Base/Atomic.h Controller/Controller.h Controller/ControllerImpl.h Logger/LogLevel.h Logger/Logger.h View/View.h 
$(OBJDIR)/Env/Env.o: Env/Env.cc Env/Env.h Env/Locale.h Env/Process.h 
$(OBJDIR)/Env/Locale.o: Env/Locale.cc Env/Locale.h Env/LocaleImpl.h 
$(OBJDIR)/Env/LocaleImpl.o: Env/LocaleImpl.cc Env/Locale.h Env/LocaleImpl.h Env/Process.h 
$(OBJDIR)/Env/Process.o: Env/Process.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/Env/ProcessImpl.o: Env/ProcessImpl.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/Logger/LogLevel.o: Logger/LogLevel.cc Logger/LogLevel.h 
$(OBJDIR)/Logger/Logger.o: Logger/Logger.cc Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h 
$(OBJDIR)/Logger/LoggerImpl.o: Logger/LoggerImpl.cc App/Constants.h Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h 
$(OBJDIR)/Model/Model.o: Model/Model.cc Logger/LogLevel.h Logger/Logger.h Model/Model.h Model/ModelImpl.h 
$(OBJDIR)/Model/ModelImpl.o: Model/ModelImpl.cc Logger/LogLevel.h Logger/Logger.h Model/Model.h Model/ModelImpl.h 
$(OBJDIR)/View/MainWindow.o: View/MainWindow.cc Controller/Controller.h View/MainWindow.h 
$(OBJDIR)/View/View.o: View/View.cc Logger/LogLevel.h Logger/Logger.h View/MainWindow.h View/View.h View/ViewImpl.h 
$(OBJDIR)/View/ViewImpl.o: View/ViewImpl.cc Logger/LogLevel.h Logger/Logger.h View/MainWindow.h View/View.h View/ViewImpl.h 
