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
View/Icon \
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
$(OBJDIR)/Base/StringBuffer.o \
$(OBJDIR)/Controller/Background.o \
$(OBJDIR)/Controller/Controller.o \
$(OBJDIR)/Controller/ControllerImpl.o \
$(OBJDIR)/Env/Env.o \
$(OBJDIR)/Env/Locale.o \
$(OBJDIR)/Env/LocaleImpl.o \
$(OBJDIR)/Env/Process.o \
$(OBJDIR)/Env/ProcessImpl.o \
$(OBJDIR)/File/Json.o \
$(OBJDIR)/File/JsonLexer.o \
$(OBJDIR)/File/JsonParser.o \
$(OBJDIR)/File/JsonWriter.o \
$(OBJDIR)/Logger/Debug.o \
$(OBJDIR)/Logger/LogLevel.o \
$(OBJDIR)/Logger/Logger.o \
$(OBJDIR)/Logger/LoggerImpl.o \
$(OBJDIR)/Logger/Trace.o \
$(OBJDIR)/Model/ConnectSpec.o \
$(OBJDIR)/Model/Model.o \
$(OBJDIR)/Model/ModelImpl.o \
$(OBJDIR)/Model/ModelImpl_load.o \
$(OBJDIR)/Model/ModelImpl_save.o \
$(OBJDIR)/Model/PatchBase.o \
$(OBJDIR)/Model/ThreadManager.o \
$(OBJDIR)/Net/AddressResolution.o \
$(OBJDIR)/Net/HostEntry.o \
$(OBJDIR)/Net/MacAddress.o \
$(OBJDIR)/Util/Base64Decoder.o \
$(OBJDIR)/Util/Base64Encoder.o \
$(OBJDIR)/Util/ByteString.o \
$(OBJDIR)/Util/Scrambler.o \
$(OBJDIR)/Util/ScramblerBase.o \
$(OBJDIR)/Util/Util.o \
$(OBJDIR)/Util/UUID.o \
$(OBJDIR)/View/ConnectDialog.o \
$(OBJDIR)/View/MainWindow.o \
$(OBJDIR)/View/PixStore.o \
$(OBJDIR)/View/ServerTreeStore.o \
$(OBJDIR)/View/ServerTreeView.o \
$(OBJDIR)/View/View.o \
$(OBJDIR)/View/ViewImpl.o \
$(OBJDIR)/XenServer/Host.o \
$(OBJDIR)/XenServer/Network.o \
$(OBJDIR)/XenServer/PhysicalBlockDevice.o \
$(OBJDIR)/XenServer/PhysicalInterface.o \
$(OBJDIR)/XenServer/Session.o \
$(OBJDIR)/XenServer/StorageRepository.o \
$(OBJDIR)/XenServer/VirtualBlockDevice.o \
$(OBJDIR)/XenServer/VirtualDiskImage.o \
$(OBJDIR)/XenServer/VirtualInterface.o \
$(OBJDIR)/XenServer/VirtualMachine.o \
$(OBJDIR)/XenServer/XenEventMonitor.o \
$(OBJDIR)/XenServer/XenObject.o \
$(OBJDIR)/XenServer/XenObjectStore.o \
$(OBJDIR)/XenServer/XenObjectTypeMap.o \
$(OBJDIR)/XenServer/XenServer.o \
$(OBJDIR)/XenServer/XenTask.o

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

$(OBJDIR)/App/Main.o: App/Main.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Controller/Controller.h Env/Env.h Env/Locale.h Logger/LogLevel.h Logger/Logger.h Model/Model.h Model/ThreadManager.h View/View.h XenServer/Api.h 
$(OBJDIR)/Base/RefObj.o: Base/RefObj.cc Base/Atomic.h Base/RefObj.h 
$(OBJDIR)/Base/StringBuffer.o: Base/StringBuffer.cc Base/StringBuffer.h 
$(OBJDIR)/Controller/Background.o: Controller/Background.cc Controller/Background.h Model/ThreadManager.h 
$(OBJDIR)/Controller/Controller.o: Controller/Controller.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Controller/ControllerImpl.h 
$(OBJDIR)/Controller/ControllerImpl.o: Controller/ControllerImpl.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Background.h Controller/Controller.h Controller/ControllerImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/ThreadManager.h Net/MacAddress.h View/View.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/PhysicalBlockDevice.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenEventMonitor.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/Env/Env.o: Env/Env.cc Env/Env.h Env/Locale.h Env/Process.h 
$(OBJDIR)/Env/Locale.o: Env/Locale.cc Env/Locale.h Env/LocaleImpl.h 
$(OBJDIR)/Env/LocaleImpl.o: Env/LocaleImpl.cc Env/Locale.h Env/LocaleImpl.h Env/Process.h 
$(OBJDIR)/Env/Process.o: Env/Process.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/Env/ProcessImpl.o: Env/ProcessImpl.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/File/Json.o: File/Json.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h File/JsonLexer.h File/JsonParser.h File/JsonWriter.h 
$(OBJDIR)/File/JsonLexer.o: File/JsonLexer.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h File/JsonLexer.h 
$(OBJDIR)/File/JsonParser.o: File/JsonParser.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h File/JsonLexer.h File/JsonParser.h 
$(OBJDIR)/File/JsonWriter.o: File/JsonWriter.cc Base/RefObj.h Base/RefPtr.h File/Json.h File/JsonWriter.h 
$(OBJDIR)/Logger/Debug.o: Logger/Debug.cc Logger/Debug.h Logger/LogLevel.h Logger/Logger.h 
$(OBJDIR)/Logger/LogLevel.o: Logger/LogLevel.cc Logger/LogLevel.h 
$(OBJDIR)/Logger/Logger.o: Logger/Logger.cc Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h 
$(OBJDIR)/Logger/LoggerImpl.o: Logger/LoggerImpl.cc App/Constants.h Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h Model/ThreadManager.h 
$(OBJDIR)/Logger/Trace.o: Logger/Trace.cc Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h 
$(OBJDIR)/Model/ConnectSpec.o: Model/ConnectSpec.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h Util/Base64.h Util/ByteString.h Util/Scrambler.h Util/ScramblerBase.h Util/UUID.h 
$(OBJDIR)/Model/Model.o: Model/Model.cc Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h Model/Model.h Model/ModelImpl.h 
$(OBJDIR)/Model/ModelImpl.o: Model/ModelImpl.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/ModelImpl.h Model/PatchBase.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenObjectTypeMap.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Model/ModelImpl_load.o: Model/ModelImpl_load.cc Base/RefObj.h Base/RefPtr.h File/Json.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/ModelImpl.h Net/MacAddress.h View/View.h 
$(OBJDIR)/Model/ModelImpl_save.o: Model/ModelImpl_save.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/ModelImpl.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Model/PatchBase.o: Model/PatchBase.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Model/Model.h Model/PatchBase.h Model/PatchRecord.h Model/PatchState.h 
$(OBJDIR)/Model/ThreadManager.o: Model/ThreadManager.cc Base/StringBuffer.h Model/ThreadManager.h 
$(OBJDIR)/Net/AddressResolution.o: Net/AddressResolution.cc Net/AddressResolution.h 
$(OBJDIR)/Net/HostEntry.o: Net/HostEntry.cc Net/HostEntry.h 
$(OBJDIR)/Net/MacAddress.o: Net/MacAddress.cc Net/AddressResolution.h Net/HostEntry.h Net/MacAddress.h 
$(OBJDIR)/Util/Base64Decoder.o: Util/Base64Decoder.cc Util/Base64.h Util/ByteString.h 
$(OBJDIR)/Util/Base64Encoder.o: Util/Base64Encoder.cc Util/Base64.h Util/ByteString.h 
$(OBJDIR)/Util/ByteString.o: Util/ByteString.cc Util/ByteString.h 
$(OBJDIR)/Util/Scrambler.o: Util/Scrambler.cc Base/RefObj.h Base/RefPtr.h Util/Scrambler.h Util/ScramblerBase.h 
$(OBJDIR)/Util/ScramblerBase.o: Util/ScramblerBase.cc Base/RefObj.h Base/RefPtr.h Util/ScramblerBase.h 
$(OBJDIR)/Util/UUID.o: Util/UUID.cc Util/UUID.h 
$(OBJDIR)/Util/Util.o: Util/Util.cc Base/StringBuffer.h Util/Util.h 
$(OBJDIR)/View/ConnectDialog.o: View/ConnectDialog.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h Util/Base64.h Util/ByteString.h Util/Scrambler.h Util/ScramblerBase.h View/ConnectDialog.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/MainWindow.o: View/MainWindow.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Logger/LogLevel.h Logger/Logger.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h View/MainWindow.h View/PixStore.h View/ServerTreeView.h View/View.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/PixStore.o: View/PixStore.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h View/Icon/CdRom.h View/Icon/HardDisk.h View/Icon/Hourglass.h View/Icon/NetworkAdapter.h View/Icon/No.h View/Icon/Pause.h View/Icon/PowerOff.h View/Icon/PowerOn.h View/Icon/RemovableMedia.h View/Icon/Warning.h View/Icon/Yes.h View/PixStore.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/ServerTreeStore.o: View/ServerTreeStore.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h View/ServerTreeStore.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/ServerTreeView.o: View/ServerTreeView.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h View/PixStore.h View/ServerTreeStore.h View/ServerTreeView.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/View.o: View/View.cc Base/RefPtr.h View/MainWindow.h View/PixStore.h View/ServerTreeView.h View/View.h View/ViewImpl.h 
$(OBJDIR)/View/ViewImpl.o: View/ViewImpl.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h View/ConnectDialog.h View/MainWindow.h View/PixStore.h View/ServerTreeView.h View/View.h View/ViewImpl.h XenServer/Api.h XenServer/Constants.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/Host.o: XenServer/Host.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/PatchBase.h Model/PatchRecord.h Model/PatchState.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/Network.o: XenServer/Network.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Network.h XenServer/Session.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/PhysicalBlockDevice.o: XenServer/PhysicalBlockDevice.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/PhysicalBlockDevice.h XenServer/Session.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/PhysicalInterface.o: XenServer/PhysicalInterface.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/Session.o: XenServer/Session.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/StorageRepository.o: XenServer/StorageRepository.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h XenServer/Api.h XenServer/Constants.h XenServer/PhysicalBlockDevice.h XenServer/Session.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualBlockDevice.o: XenServer/VirtualBlockDevice.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualDiskImage.o: XenServer/VirtualDiskImage.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/VirtualInterface.o: XenServer/VirtualInterface.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualMachine.o: XenServer/VirtualMachine.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Macros.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/XenEventMonitor.o: XenServer/XenEventMonitor.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/PhysicalBlockDevice.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenEventMonitor.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenObjectTypeMap.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/XenObject.o: XenServer/XenObject.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/XenObjectStore.o: XenServer/XenObjectStore.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Macros.h XenServer/Network.h XenServer/PhysicalBlockDevice.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/XenObjectTypeMap.o: XenServer/XenObjectTypeMap.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h XenServer/Api.h XenServer/Constants.h XenServer/XenObject.h XenServer/XenObjectTypeMap.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/XenServer.o: XenServer/XenServer.cc Base/RefObj.h Base/StringBuffer.h Util/Util.h XenServer/Api.h XenServer/Constants.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/XenTask.o: XenServer/XenTask.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
