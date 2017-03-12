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
Net \
Util \
View \
XenServer

clean distclean::
	for d in $(SUBDIRS); do $(RM) $$d/*~ ; done

######################################################################

PROJ1=$(BINDIR)/$(TARGETEXE)
LIBS1=$(STDLIBS) $(USRLIBS)
OBJS1=$(OBJDIR)/App/Main.o \
$(OBJDIR)/Base/InterlockedCompareExchange.o \
$(OBJDIR)/Base/InterlockedExchange.o \
$(OBJDIR)/Base/InterlockedExchangeAdd.o \
$(OBJDIR)/Base/RefObj.o \
$(OBJDIR)/Controller/Background.o \
$(OBJDIR)/Controller/Controller.o \
$(OBJDIR)/Controller/ControllerImpl.o \
$(OBJDIR)/Env/Env.o \
$(OBJDIR)/Env/Locale.o \
$(OBJDIR)/Env/LocaleImpl.o \
$(OBJDIR)/Env/Process.o \
$(OBJDIR)/Env/ProcessImpl.o \
$(OBJDIR)/Logger/Debug.o \
$(OBJDIR)/Logger/LogLevel.o \
$(OBJDIR)/Logger/Logger.o \
$(OBJDIR)/Logger/LoggerImpl.o \
$(OBJDIR)/Logger/Trace.o \
$(OBJDIR)/Model/ConnectSpec.o \
$(OBJDIR)/Model/Json.o \
$(OBJDIR)/Model/JsonLexer.o \
$(OBJDIR)/Model/JsonParser.o \
$(OBJDIR)/Model/Model.o \
$(OBJDIR)/Model/ModelImpl.o \
$(OBJDIR)/Model/ModelImpl_load.o \
$(OBJDIR)/Model/ModelImpl_save.o \
$(OBJDIR)/Net/AddressResolution.o \
$(OBJDIR)/Net/HostEntry.o \
$(OBJDIR)/Net/MacAddress.o \
$(OBJDIR)/Util/Base64Decoder.o \
$(OBJDIR)/Util/Base64Encoder.o \
$(OBJDIR)/Util/ByteString.o \
$(OBJDIR)/Util/Scrambler.o \
$(OBJDIR)/Util/ScramblerBase.o \
$(OBJDIR)/Util/StringBuffer.o \
$(OBJDIR)/Util/UUID.o \
$(OBJDIR)/View/MainWindow.o \
$(OBJDIR)/View/View.o \
$(OBJDIR)/View/ViewImpl.o \
$(OBJDIR)/XenServer/Session.o \
$(OBJDIR)/XenServer/XenObject.o \
$(OBJDIR)/XenServer/XenObjectStore.o

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

$(OBJDIR)/App/Main.o: App/Main.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Controller/Controller.h Env/Env.h Env/Locale.h Logger/LogLevel.h Logger/Logger.h Model/Model.h View/View.h XenServer/Api.h 
$(OBJDIR)/Base/RefObj.o: Base/RefObj.cc Base/Atomic.h Base/RefObj.h 
$(OBJDIR)/Controller/Background.o: Controller/Background.cc Base/RefObj.h Base/RefPtr.h Controller/Background.h Controller/Controller.h 
$(OBJDIR)/Controller/Controller.o: Controller/Controller.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Controller/ControllerImpl.h 
$(OBJDIR)/Controller/ControllerImpl.o: Controller/ControllerImpl.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Controller/Controller.h Controller/ControllerImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h View/View.h 
$(OBJDIR)/Env/Env.o: Env/Env.cc Env/Env.h Env/Locale.h Env/Process.h 
$(OBJDIR)/Env/Locale.o: Env/Locale.cc Env/Locale.h Env/LocaleImpl.h 
$(OBJDIR)/Env/LocaleImpl.o: Env/LocaleImpl.cc Env/Locale.h Env/LocaleImpl.h Env/Process.h 
$(OBJDIR)/Env/Process.o: Env/Process.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/Env/ProcessImpl.o: Env/ProcessImpl.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/Logger/Debug.o: Logger/Debug.cc Logger/Debug.h Logger/LogLevel.h Logger/Logger.h 
$(OBJDIR)/Logger/LogLevel.o: Logger/LogLevel.cc Logger/LogLevel.h 
$(OBJDIR)/Logger/Logger.o: Logger/Logger.cc Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h 
$(OBJDIR)/Logger/LoggerImpl.o: Logger/LoggerImpl.cc App/Constants.h Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h 
$(OBJDIR)/Logger/Trace.o: Logger/Trace.cc Logger/LogLevel.h Logger/Logger.h Logger/Trace.h 
$(OBJDIR)/Model/ConnectSpec.o: Model/ConnectSpec.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h Util/Base64.h Util/ByteString.h Util/Scrambler.h Util/ScramblerBase.h Util/StringBuffer.h Util/UUID.h 
$(OBJDIR)/Model/Json.o: Model/Json.cc Base/RefObj.h Base/RefPtr.h Model/Json.h Model/JsonLexer.h Model/JsonParser.h 
$(OBJDIR)/Model/JsonLexer.o: Model/JsonLexer.cc Base/RefObj.h Base/RefPtr.h Model/Json.h Model/JsonLexer.h 
$(OBJDIR)/Model/JsonParser.o: Model/JsonParser.cc Base/RefObj.h Base/RefPtr.h Model/Json.h Model/JsonLexer.h Model/JsonParser.h 
$(OBJDIR)/Model/Model.o: Model/Model.cc Logger/LogLevel.h Logger/Logger.h Model/Model.h Model/ModelImpl.h 
$(OBJDIR)/Model/ModelImpl.o: Model/ModelImpl.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/ModelImpl.h Net/MacAddress.h XenServer/Api.h XenServer/Session.h XenServer/XenObject.h 
$(OBJDIR)/Model/ModelImpl_load.o: Model/ModelImpl_load.cc Base/RefObj.h Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/Json.h Model/Model.h Model/ModelImpl.h View/View.h 
$(OBJDIR)/Model/ModelImpl_save.o: Model/ModelImpl_save.cc Base/RefObj.h Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/Json.h Model/Model.h Model/ModelImpl.h 
$(OBJDIR)/Net/AddressResolution.o: Net/AddressResolution.cc Net/AddressResolution.h 
$(OBJDIR)/Net/HostEntry.o: Net/HostEntry.cc Net/HostEntry.h 
$(OBJDIR)/Net/MacAddress.o: Net/MacAddress.cc Net/AddressResolution.h Net/HostEntry.h Net/MacAddress.h 
$(OBJDIR)/Util/Base64Decoder.o: Util/Base64Decoder.cc Util/Base64.h Util/ByteString.h 
$(OBJDIR)/Util/Base64Encoder.o: Util/Base64Encoder.cc Util/Base64.h Util/ByteString.h 
$(OBJDIR)/Util/ByteString.o: Util/ByteString.cc Util/ByteString.h 
$(OBJDIR)/Util/Scrambler.o: Util/Scrambler.cc Base/RefObj.h Base/RefPtr.h Util/Scrambler.h Util/ScramblerBase.h 
$(OBJDIR)/Util/ScramblerBase.o: Util/ScramblerBase.cc Base/RefObj.h Base/RefPtr.h Util/ScramblerBase.h 
$(OBJDIR)/Util/StringBuffer.o: Util/StringBuffer.cc Util/StringBuffer.h 
$(OBJDIR)/Util/UUID.o: Util/UUID.cc Util/UUID.h 
$(OBJDIR)/View/MainWindow.o: View/MainWindow.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/MainWindow.h 
$(OBJDIR)/View/View.o: View/View.cc View/MainWindow.h View/View.h View/ViewImpl.h 
$(OBJDIR)/View/ViewImpl.o: View/ViewImpl.cc Logger/LogLevel.h Logger/Logger.h Logger/Trace.h View/MainWindow.h View/View.h View/ViewImpl.h 
$(OBJDIR)/XenServer/Session.o: XenServer/Session.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h 
$(OBJDIR)/XenServer/XenObject.o: XenServer/XenObject.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Controller/Controller.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h 
$(OBJDIR)/XenServer/XenObjectStore.o: XenServer/XenObjectStore.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Model/Model.h XenServer/Api.h XenServer/Constants.h XenServer/Macros.h XenServer/XenObject.h XenServer/XenObjectStore.h 
