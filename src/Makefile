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
Exception \
File \
Logger \
Model \
Net \
Protocol \
Thread \
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
$(OBJDIR)/Controller/Controller.o \
$(OBJDIR)/Controller/ControllerImpl.o \
$(OBJDIR)/Controller/ControllerImpl_host.o \
$(OBJDIR)/Controller/ControllerImpl_sr.o \
$(OBJDIR)/Controller/ControllerImpl_vdi.o \
$(OBJDIR)/Controller/ControllerImpl_vm.o \
$(OBJDIR)/Controller/SignalManager.o \
$(OBJDIR)/Controller/SignalManagerImpl.o \
$(OBJDIR)/Env/Env.o \
$(OBJDIR)/Env/Locale.o \
$(OBJDIR)/Env/LocaleImpl.o \
$(OBJDIR)/Env/Process.o \
$(OBJDIR)/Env/ProcessImpl.o \
$(OBJDIR)/Exception/Exception.o \
$(OBJDIR)/Exception/ConsoleException.o \
$(OBJDIR)/File/File.o \
$(OBJDIR)/File/FileImpl.o \
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
$(OBJDIR)/Model/ConsoleInfo.o \
$(OBJDIR)/Model/Model.o \
$(OBJDIR)/Model/ModelImpl.o \
$(OBJDIR)/Model/ModelImpl_load.o \
$(OBJDIR)/Model/ModelImpl_save.o \
$(OBJDIR)/Model/PatchBase.o \
$(OBJDIR)/Net/AddressResolution.o \
$(OBJDIR)/Net/Console.o \
$(OBJDIR)/Net/ConsoleConnector.o \
$(OBJDIR)/Net/ConsoleImpl.o \
$(OBJDIR)/Net/HostEntry.o \
$(OBJDIR)/Net/MacAddress.o \
$(OBJDIR)/Net/NetRoute.o \
$(OBJDIR)/Net/PingAgent.o \
$(OBJDIR)/Net/PingAgentImpl.o \
$(OBJDIR)/Net/UdpSocket.o \
$(OBJDIR)/Net/WakeOnLan.o \
$(OBJDIR)/Thread/ThreadManager.o \
$(OBJDIR)/Thread/ThreadManagerImpl.o \
$(OBJDIR)/Util/Base64Decoder.o \
$(OBJDIR)/Util/Base64Encoder.o \
$(OBJDIR)/Util/ByteString.o \
$(OBJDIR)/Util/Scrambler.o \
$(OBJDIR)/Util/ScramblerBase.o \
$(OBJDIR)/Util/Util.o \
$(OBJDIR)/Util/UUID.o \
$(OBJDIR)/View/AboutDialog.o \
$(OBJDIR)/View/AddCifsDialog.o \
$(OBJDIR)/View/AddVmDialog.o \
$(OBJDIR)/View/AttachCdDialog.o \
$(OBJDIR)/View/AttachHddDialog.o \
$(OBJDIR)/View/AttachNicDialog.o \
$(OBJDIR)/View/BlockDeviceNumberComboBox.o \
$(OBJDIR)/View/CdDeviceComboBox.o \
$(OBJDIR)/View/CdImageListView.o \
$(OBJDIR)/View/ChangeCdDialog.o \
$(OBJDIR)/View/ConnectDialog.o \
$(OBJDIR)/View/ConsoleViewImpl.o \
$(OBJDIR)/View/ConsoleViewKeyboardInputFilterImpl.o \
$(OBJDIR)/View/CopyVmDialog.o \
$(OBJDIR)/View/CpuDialog.o \
$(OBJDIR)/View/CpuGraph.o \
$(OBJDIR)/View/DeleteVmDialog.o \
$(OBJDIR)/View/DeviceNumberComboBox.o \
$(OBJDIR)/View/ExportVmDialog.o \
$(OBJDIR)/View/FrameBuffer.o \
$(OBJDIR)/View/FrameBuffer24.o \
$(OBJDIR)/View/FrameBuffer32.o \
$(OBJDIR)/View/FrameScalerImpl.o \
$(OBJDIR)/View/HardDiskDriveListBox.o \
$(OBJDIR)/View/HardDiskDriveListView.o \
$(OBJDIR)/View/HardDiskDriveSpecDialog.o \
$(OBJDIR)/View/HostMenu.o \
$(OBJDIR)/View/HostNotebook.o \
$(OBJDIR)/View/HostTreeStore.o \
$(OBJDIR)/View/HostTreeView.o \
$(OBJDIR)/View/ImportVmDialog.o \
$(OBJDIR)/View/InputOutputGraph.o \
$(OBJDIR)/View/MainWindow.o \
$(OBJDIR)/View/MemoryDialog.o \
$(OBJDIR)/View/MemoryGraph.o \
$(OBJDIR)/View/NameDialog.o \
$(OBJDIR)/View/NameValueListView.o \
$(OBJDIR)/View/NetworkDeviceNumberComboBox.o \
$(OBJDIR)/View/NetworkListView.o \
$(OBJDIR)/View/NetworkMenu.o \
$(OBJDIR)/View/NetworkNotebook.o \
$(OBJDIR)/View/NoContentsNotebook.o \
$(OBJDIR)/View/NotebookFactory.o \
$(OBJDIR)/View/NotebookStore.o \
$(OBJDIR)/View/PatchListView.o \
$(OBJDIR)/View/PatchMenu.o \
$(OBJDIR)/View/PixStore.o \
$(OBJDIR)/View/PropertyHelper.o \
$(OBJDIR)/View/ResizeDialog.o \
$(OBJDIR)/View/ResourceHistoryGraph.o \
$(OBJDIR)/View/SizeInBytesBox.o \
$(OBJDIR)/View/ShadowMemoryDialog.o \
$(OBJDIR)/View/SnapshotMenu.o \
$(OBJDIR)/View/SnapshotTreeView.o \
$(OBJDIR)/View/SnapshotView.o \
$(OBJDIR)/View/StorageRepositoryComboBox.o \
$(OBJDIR)/View/StorageRepositoryMenu.o \
$(OBJDIR)/View/StorageRepositoryNotebook.o \
$(OBJDIR)/View/VgaDialog.o \
$(OBJDIR)/View/VirtualBlockDeviceMenu.o \
$(OBJDIR)/View/VirtualDiskImageMenu.o \
$(OBJDIR)/View/VirtualDiskImageListView.o \
$(OBJDIR)/View/VirtualInterfaceMenu.o \
$(OBJDIR)/View/VirtualMachineGenMenu.o \
$(OBJDIR)/View/VirtualMachineMemMenu.o \
$(OBJDIR)/View/VirtualMachineMenu.o \
$(OBJDIR)/View/VirtualMachineNotebook.o \
$(OBJDIR)/View/VirtualMachinePropertyView.o \
$(OBJDIR)/View/VirtualMachineStatusWindow.o \
$(OBJDIR)/View/VirtualMachineTemplateListView.o \
$(OBJDIR)/View/View.o \
$(OBJDIR)/View/ViewImpl.o \
$(OBJDIR)/XenServer/CifsSpec.o \
$(OBJDIR)/XenServer/GetNotificationText.o \
$(OBJDIR)/XenServer/GetXenObjectTypeText.o \
$(OBJDIR)/XenServer/HardDiskDriveSpec.o \
$(OBJDIR)/XenServer/Host.o \
$(OBJDIR)/XenServer/Network.o \
$(OBJDIR)/XenServer/PerformanceMonitor.o \
$(OBJDIR)/XenServer/PhysicalBlockDevice.o \
$(OBJDIR)/XenServer/PhysicalInterface.o \
$(OBJDIR)/XenServer/Session.o \
$(OBJDIR)/XenServer/StorageRepository.o \
$(OBJDIR)/XenServer/VirtualBlockDevice.o \
$(OBJDIR)/XenServer/VirtualDiskImage.o \
$(OBJDIR)/XenServer/VirtualInterface.o \
$(OBJDIR)/XenServer/VirtualMachine.o \
$(OBJDIR)/XenServer/VirtualMachineArchive.o \
$(OBJDIR)/XenServer/VirtualMachineExporter.o \
$(OBJDIR)/XenServer/VirtualMachineImporter.o \
$(OBJDIR)/XenServer/VirtualMachinePorter.o \
$(OBJDIR)/XenServer/VirtualMachineVerifier.o \
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

##### dependency begin #####
$(OBJDIR)/App/Main.o: App/Main.cc App/Constants.h Base/RefPtr.h Controller/Controller.h Env/Env.h Env/Locale.h Logger/LogLevel.h Logger/Logger.h Model/Model.h Net/PingAgent.h Thread/ThreadManager.h View/View.h XenServer/Api.h 
$(OBJDIR)/Base/RefObj.o: Base/RefObj.cc Base/Atomic.h Base/RefObj.h Logger/LogLevel.h Logger/Logger.h 
$(OBJDIR)/Base/StringBuffer.o: Base/StringBuffer.cc Base/StringBuffer.h 
$(OBJDIR)/Controller/Controller.o: Controller/Controller.cc Base/RefPtr.h Controller/Controller.h Controller/ControllerImpl.h Controller/SignalManager.h XenServer/CifsSpec.h XenServer/HardDiskDriveSpec.h XenServer/VirtualMachineSpec.h 
$(OBJDIR)/Controller/ControllerImpl.o: Controller/ControllerImpl.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Controller/ControllerImpl.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h Net/PingAgent.h Thread/ThreadManager.h View/View.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Host.h XenServer/Session.h XenServer/VirtualMachineSpec.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/Controller/ControllerImpl_host.o: Controller/ControllerImpl_host.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Controller/ControllerImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h Net/WakeOnLan.h Thread/ThreadManager.h View/View.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Host.h XenServer/Network.h XenServer/PerformanceMonitor.h XenServer/PhysicalBlockDevice.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/VirtualMachineSpec.h XenServer/XenEventMonitor.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Controller/ControllerImpl_sr.o: Controller/ControllerImpl_sr.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Controller/ControllerImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h Thread/ThreadManager.h View/View.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Host.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualMachineSpec.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Controller/ControllerImpl_vdi.o: Controller/ControllerImpl_vdi.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Controller/ControllerImpl.h Model/ConnectSpec.h Net/MacAddress.h View/View.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Session.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/VirtualMachineSpec.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Controller/ControllerImpl_vm.o: Controller/ControllerImpl_vm.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Controller/ControllerImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/Console.h Net/MacAddress.h Thread/ThreadManager.h View/View.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Host.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/VirtualMachineExporter.h XenServer/VirtualMachineImporter.h XenServer/VirtualMachineOperationState.h XenServer/VirtualMachinePorter.h XenServer/VirtualMachineSpec.h XenServer/VirtualMachineVerifier.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Controller/SignalManager.o: Controller/SignalManager.cc Base/RefPtr.h Controller/SignalManager.h Controller/SignalManagerImpl.h 
$(OBJDIR)/Controller/SignalManagerImpl.o: Controller/SignalManagerImpl.cc Base/RefObj.h Base/RefPtr.h Controller/SignalManager.h Controller/SignalManagerImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Thread/ThreadManager.h XenServer/Api.h XenServer/Constants.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Env/Env.o: Env/Env.cc Env/Env.h Env/Locale.h Env/Process.h 
$(OBJDIR)/Env/Locale.o: Env/Locale.cc Env/Locale.h Env/LocaleImpl.h 
$(OBJDIR)/Env/LocaleImpl.o: Env/LocaleImpl.cc Env/Locale.h Env/LocaleImpl.h Env/Process.h 
$(OBJDIR)/Env/Process.o: Env/Process.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/Env/ProcessImpl.o: Env/ProcessImpl.cc Env/Process.h Env/ProcessImpl.h 
$(OBJDIR)/Exception/ConsoleException.o: Exception/ConsoleException.cc Base/StringBuffer.h Exception/ConsoleException.h Exception/Exception.h 
$(OBJDIR)/Exception/Exception.o: Exception/Exception.cc Base/StringBuffer.h Exception/Exception.h 
$(OBJDIR)/File/File.o: File/File.cc Base/RefObj.h Base/RefPtr.h File/File.h File/FileImpl.h 
$(OBJDIR)/File/FileImpl.o: File/FileImpl.cc Base/RefObj.h Base/RefPtr.h File/File.h File/FileImpl.h 
$(OBJDIR)/File/Json.o: File/Json.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h File/JsonLexer.h File/JsonParser.h File/JsonWriter.h 
$(OBJDIR)/File/JsonLexer.o: File/JsonLexer.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h File/JsonLexer.h 
$(OBJDIR)/File/JsonParser.o: File/JsonParser.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h File/JsonLexer.h File/JsonParser.h 
$(OBJDIR)/File/JsonWriter.o: File/JsonWriter.cc Base/RefObj.h Base/RefPtr.h File/Json.h File/JsonWriter.h 
$(OBJDIR)/Logger/Debug.o: Logger/Debug.cc Logger/Debug.h Logger/LogLevel.h Logger/Logger.h 
$(OBJDIR)/Logger/LogLevel.o: Logger/LogLevel.cc Logger/LogLevel.h 
$(OBJDIR)/Logger/Logger.o: Logger/Logger.cc Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h 
$(OBJDIR)/Logger/LoggerImpl.o: Logger/LoggerImpl.cc App/Constants.h Logger/LogLevel.h Logger/Logger.h Logger/LoggerImpl.h Thread/ThreadManager.h 
$(OBJDIR)/Logger/Trace.o: Logger/Trace.cc Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h 
$(OBJDIR)/Model/ConnectSpec.o: Model/ConnectSpec.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h Model/ConnectSpec.h Net/MacAddress.h Util/Base64.h Util/ByteString.h Util/Scrambler.h Util/ScramblerBase.h Util/UUID.h 
$(OBJDIR)/Model/ConsoleInfo.o: Model/ConsoleInfo.cc Base/RefObj.h Base/RefPtr.h File/Json.h Model/ConsoleInfo.h Net/Console.h 
$(OBJDIR)/Model/Model.o: Model/Model.cc Base/RefObj.h Base/RefPtr.h File/Json.h Logger/LogLevel.h Logger/Logger.h Model/ConsoleInfo.h Model/Model.h Model/ModelImpl.h XenServer/CifsSpec.h 
$(OBJDIR)/Model/ModelImpl.o: Model/ModelImpl.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/Json.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/ConsoleInfo.h Model/Model.h Model/ModelImpl.h Model/PatchBase.h Net/Console.h Net/MacAddress.h Net/PingAgent.h Util/UUID.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenObjectTypeMap.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Model/ModelImpl_load.o: Model/ModelImpl_load.cc Base/RefObj.h Base/RefPtr.h File/Json.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/ConsoleInfo.h Model/Model.h Model/ModelImpl.h Model/PatchBase.h Net/Console.h Net/MacAddress.h View/View.h XenServer/CifsSpec.h 
$(OBJDIR)/Model/ModelImpl_save.o: Model/ModelImpl_save.cc Base/RefObj.h Base/RefPtr.h File/Json.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/ConsoleInfo.h Model/Model.h Model/ModelImpl.h Net/MacAddress.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/Model/PatchBase.o: Model/PatchBase.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/Model.h Model/PatchBase.h Model/PatchRecord.h Model/PatchState.h 
$(OBJDIR)/Net/AddressResolution.o: Net/AddressResolution.cc Net/AddressResolution.h 
$(OBJDIR)/Net/Console.o: Net/Console.cc Base/Atomic.h Base/ByteBuffer.h Base/RefObj.h Base/RefPtr.h Net/Console.h Net/ConsoleConnector.h Net/ConsoleImpl.h Protocol/Rfb.h 
$(OBJDIR)/Net/ConsoleConnector.o: Net/ConsoleConnector.cc Base/Atomic.h Base/ByteBuffer.h Base/StringBuffer.h Exception/ConsoleException.h Exception/Exception.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Net/ConsoleConnector.h 
$(OBJDIR)/Net/ConsoleImpl.o: Net/ConsoleImpl.cc Base/Atomic.h Base/ByteBuffer.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Exception/ConsoleException.h Exception/Exception.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Net/Console.h Net/ConsoleConnector.h Net/ConsoleImpl.h Protocol/Rfb.h Thread/ThreadManager.h View/ConsoleView.h 
$(OBJDIR)/Net/HostEntry.o: Net/HostEntry.cc Net/HostEntry.h 
$(OBJDIR)/Net/MacAddress.o: Net/MacAddress.cc Net/AddressResolution.h Net/HostEntry.h Net/MacAddress.h 
$(OBJDIR)/Net/NetRoute.o: Net/NetRoute.cc Net/NetRoute.h 
$(OBJDIR)/Net/PingAgent.o: Net/PingAgent.cc Net/PingAgent.h Net/PingAgentImpl.h 
$(OBJDIR)/Net/PingAgentImpl.o: Net/PingAgentImpl.cc Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Net/PingAgent.h Net/PingAgentImpl.h Thread/ThreadManager.h 
$(OBJDIR)/Net/UdpSocket.o: Net/UdpSocket.cc Net/UdpSocket.h 
$(OBJDIR)/Net/WakeOnLan.o: Net/WakeOnLan.cc Net/HostEntry.h Net/MacAddress.h Net/NetRoute.h Net/UdpSocket.h Net/WakeOnLan.h 
$(OBJDIR)/Thread/ThreadManager.o: Thread/ThreadManager.cc Thread/ThreadManager.h Thread/ThreadManagerImpl.h 
$(OBJDIR)/Thread/ThreadManagerImpl.o: Thread/ThreadManagerImpl.cc Base/StringBuffer.h Thread/ThreadManager.h Thread/ThreadManagerImpl.h 
$(OBJDIR)/Util/Base64Decoder.o: Util/Base64Decoder.cc Util/Base64.h Util/ByteString.h 
$(OBJDIR)/Util/Base64Encoder.o: Util/Base64Encoder.cc Util/Base64.h Util/ByteString.h 
$(OBJDIR)/Util/ByteString.o: Util/ByteString.cc Util/ByteString.h 
$(OBJDIR)/Util/Scrambler.o: Util/Scrambler.cc Base/RefObj.h Base/RefPtr.h Util/Scrambler.h Util/ScramblerBase.h 
$(OBJDIR)/Util/ScramblerBase.o: Util/ScramblerBase.cc Base/RefObj.h Base/RefPtr.h Util/ScramblerBase.h 
$(OBJDIR)/Util/UUID.o: Util/UUID.cc Util/UUID.h 
$(OBJDIR)/Util/Util.o: Util/Util.cc Base/StringBuffer.h Util/Util.h 
$(OBJDIR)/View/AboutDialog.o: View/AboutDialog.cc App/Constants.h App/License.h App/Version.h View/AboutDialog.h View/PixStore.h 
$(OBJDIR)/View/AddCifsDialog.o: View/AddCifsDialog.cc Base/RefObj.h Base/RefPtr.h Util/Base64.h Util/ByteString.h Util/Scrambler.h Util/ScramblerBase.h View/AddCifsDialog.h XenServer/CifsSpec.h 
$(OBJDIR)/View/AddVmDialog.o: View/AddVmDialog.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/AddVmDialog.h View/CdImageListView.h View/HardDiskDriveListBox.h View/HardDiskDriveListView.h View/NetworkListView.h View/VirtualMachineTemplateListView.h XenServer/Api.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Session.h XenServer/VirtualMachine.h XenServer/VirtualMachineSpec.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/AttachCdDialog.o: View/AttachCdDialog.cc Base/RefObj.h Base/RefPtr.h View/AttachCdDialog.h View/BlockDeviceNumberComboBox.h View/DeviceNumberComboBox.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/AttachHddDialog.o: View/AttachHddDialog.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Controller/SignalManager.h Model/ConnectSpec.h Net/MacAddress.h View/AttachHddDialog.h View/BlockDeviceNumberComboBox.h View/DeviceNumberComboBox.h View/NameValueMenu.h View/StorageRepositoryComboBox.h View/VirtualDiskImageListView.h View/VirtualDiskImageMenu.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/AttachNicDialog.o: View/AttachNicDialog.cc Base/RefObj.h Base/RefPtr.h View/AttachNicDialog.h View/DeviceNumberComboBox.h View/NetworkDeviceNumberComboBox.h View/NetworkListView.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/BlockDeviceNumberComboBox.o: View/BlockDeviceNumberComboBox.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/BlockDeviceNumberComboBox.h View/DeviceNumberComboBox.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/CdDeviceComboBox.o: View/CdDeviceComboBox.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/CdDeviceComboBox.h XenServer/Api.h XenServer/Constants.h XenServer/OpticalDevice.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/CdImageListView.o: View/CdImageListView.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h View/CdImageListView.h XenServer/Api.h XenServer/Constants.h XenServer/OpticalDevice.h XenServer/OpticalDisk.h XenServer/PhysicalBlockDevice.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualDiskImage.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/ChangeCdDialog.o: View/ChangeCdDialog.cc Base/RefObj.h Base/RefPtr.h View/CdDeviceComboBox.h View/CdImageListView.h View/ChangeCdDialog.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualBlockDevice.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/ConnectDialog.o: View/ConnectDialog.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h Util/Base64.h Util/ByteString.h Util/Scrambler.h Util/ScramblerBase.h View/ConnectDialog.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/ConsoleViewImpl.o: View/ConsoleViewImpl.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Net/Console.h Thread/ThreadManager.h View/ConsoleView.h View/ConsoleViewImpl.h View/ConsoleViewKeyboardInputFilter.h View/FrameBuffer.h View/FrameScaler.h View/FrameScalerImpl.h 
$(OBJDIR)/View/ConsoleViewKeyboardInputFilterImpl.o: View/ConsoleViewKeyboardInputFilterImpl.cc Base/RefObj.h Base/RefPtr.h View/ConsoleViewKeyboardInputFilter.h View/ConsoleViewKeyboardInputFilterImpl.h 
$(OBJDIR)/View/CopyVmDialog.o: View/CopyVmDialog.cc Base/RefPtr.h Base/StringBuffer.h Model/Model.h Util/Util.h View/CopyVmDialog.h View/StorageRepositoryComboBox.h 
$(OBJDIR)/View/CpuDialog.o: View/CpuDialog.cc View/CpuDialog.h 
$(OBJDIR)/View/CpuGraph.o: View/CpuGraph.cc View/CpuGraph.h View/ResourceHistoryGraph.h 
$(OBJDIR)/View/DeleteVmDialog.o: View/DeleteVmDialog.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h Util/Util.h View/DeleteVmDialog.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/DeviceNumberComboBox.o: View/DeviceNumberComboBox.cc View/DeviceNumberComboBox.h 
$(OBJDIR)/View/ExportVmDialog.o: View/ExportVmDialog.cc View/ExportVmDialog.h 
$(OBJDIR)/View/FrameBuffer.o: View/FrameBuffer.cc Base/RefObj.h Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h View/FrameBuffer.h View/FrameBuffer24.h View/FrameBuffer32.h 
$(OBJDIR)/View/FrameBuffer24.o: View/FrameBuffer24.cc Base/RefObj.h Base/RefPtr.h View/FrameBuffer.h View/FrameBuffer24.h 
$(OBJDIR)/View/FrameBuffer32.o: View/FrameBuffer32.cc Base/RefObj.h Base/RefPtr.h View/FrameBuffer.h View/FrameBuffer32.h 
$(OBJDIR)/View/FrameScalerImpl.o: View/FrameScalerImpl.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Thread/ThreadManager.h View/FrameBuffer.h View/FrameScaler.h View/FrameScalerImpl.h 
$(OBJDIR)/View/HardDiskDriveListBox.o: View/HardDiskDriveListBox.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h View/HardDiskDriveListBox.h View/HardDiskDriveListView.h View/HardDiskDriveSpecDialog.h View/SizeInBytesBox.h View/StorageRepositoryComboBox.h XenServer/Api.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/HardDiskDriveListView.o: View/HardDiskDriveListView.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h View/HardDiskDriveListView.h XenServer/Api.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/HardDiskDriveSpecDialog.o: View/HardDiskDriveSpecDialog.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/HardDiskDriveSpecDialog.h View/SizeInBytesBox.h View/StorageRepositoryComboBox.h XenServer/Api.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Session.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/HostMenu.o: View/HostMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Model/ConnectSpec.h Net/MacAddress.h Net/PingAgent.h View/HostMenu.h View/NameValueMenu.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/HostNotebook.o: View/HostNotebook.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/PatchBase.h Model/PatchRecord.h Model/PatchState.h Net/MacAddress.h View/CpuGraph.h View/HostMenu.h View/HostNotebook.h View/InputOutputGraph.h View/MemoryGraph.h View/NameValueListView.h View/NameValueMenu.h View/Notebook.h View/PatchListView.h View/PatchMenu.h View/PropertyHelper.h View/ResourceHistoryGraph.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/PerformanceMonitor.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/HostTreeStore.o: View/HostTreeStore.cc Base/RefObj.h Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h View/HostTreeStore.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/HostTreeView.o: View/HostTreeView.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h View/HostMenu.h View/HostTreeStore.h View/HostTreeView.h View/NameValueMenu.h View/PixStore.h View/StorageRepositoryMenu.h View/VirtualMachineMenu.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/ImportVmDialog.o: View/ImportVmDialog.cc Logger/LogLevel.h Logger/Logger.h Logger/Trace.h View/ImportVmDialog.h 
$(OBJDIR)/View/InputOutputGraph.o: View/InputOutputGraph.cc Base/StringBuffer.h View/InputOutputGraph.h View/ResourceHistoryGraph.h 
$(OBJDIR)/View/MainWindow.o: View/MainWindow.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Controller/Controller.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h Net/PingAgent.h View/HostMenu.h View/HostTreeView.h View/MainWindow.h View/NameValueMenu.h View/Notebook.h View/NotebookFactory.h View/NotebookStore.h View/PixStore.h View/StorageRepositoryMenu.h View/View.h View/VirtualMachineMenu.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/PerformanceMonitor.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/MemoryDialog.o: View/MemoryDialog.cc View/MemoryDialog.h View/SizeInBytesBox.h 
$(OBJDIR)/View/MemoryGraph.o: View/MemoryGraph.cc Base/StringBuffer.h View/MemoryGraph.h View/ResourceHistoryGraph.h 
$(OBJDIR)/View/NameDialog.o: View/NameDialog.cc View/NameDialog.h 
$(OBJDIR)/View/NameValueListView.o: View/NameValueListView.cc View/NameValueListView.h View/NameValueMenu.h XenServer/Api.h 
$(OBJDIR)/View/NetworkDeviceNumberComboBox.o: View/NetworkDeviceNumberComboBox.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/DeviceNumberComboBox.h View/NetworkDeviceNumberComboBox.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/NetworkListView.o: View/NetworkListView.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/NetworkListView.h XenServer/Api.h XenServer/Constants.h XenServer/Network.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/NetworkMenu.o: View/NetworkMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/NameValueMenu.h View/NetworkMenu.h XenServer/Api.h XenServer/Constants.h XenServer/Network.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/NetworkNotebook.o: View/NetworkNotebook.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h View/NameValueListView.h View/NameValueMenu.h View/NetworkMenu.h View/NetworkNotebook.h View/Notebook.h View/PropertyHelper.h XenServer/Api.h XenServer/Constants.h XenServer/Network.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/NoContentsNotebook.o: View/NoContentsNotebook.cc Base/RefObj.h Base/RefPtr.h View/NoContentsNotebook.h View/Notebook.h 
$(OBJDIR)/View/NotebookFactory.o: View/NotebookFactory.cc App/Constants.h App/Version.h Base/RefObj.h Base/RefPtr.h Model/PatchRecord.h Model/PatchState.h View/CpuGraph.h View/HostMenu.h View/HostNotebook.h View/InputOutputGraph.h View/MemoryGraph.h View/NameValueListView.h View/NameValueMenu.h View/NetworkMenu.h View/NetworkNotebook.h View/NoContentsNotebook.h View/Notebook.h View/NotebookFactory.h View/PatchListView.h View/PatchMenu.h View/ResourceHistoryGraph.h View/SnapshotMenu.h View/SnapshotTreeView.h View/SnapshotView.h View/StorageRepositoryMenu.h View/StorageRepositoryNotebook.h View/VirtualBlockDeviceMenu.h View/VirtualDiskImageListView.h View/VirtualDiskImageMenu.h View/VirtualInterfaceMenu.h View/VirtualMachineGenMenu.h View/VirtualMachineMemMenu.h View/VirtualMachineNotebook.h View/VirtualMachinePropertyView.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/StorageRepository.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/NotebookStore.o: View/NotebookStore.cc Base/RefObj.h Base/RefPtr.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h View/Notebook.h View/NotebookStore.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/PatchListView.o: View/PatchListView.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/Model.h Model/PatchBase.h Model/PatchRecord.h Model/PatchState.h View/PatchListView.h View/PatchMenu.h XenServer/Api.h 
$(OBJDIR)/View/PatchMenu.o: View/PatchMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Model/Model.h Model/PatchState.h View/PatchMenu.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/PixStore.o: View/PixStore.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/Icon/CdRom.h View/Icon/Computer.h View/Icon/HardDisk.h View/Icon/Hourglass.h View/Icon/Memory.h View/Icon/NetworkAdapter.h View/Icon/No.h View/Icon/Pause.h View/Icon/PowerOff.h View/Icon/PowerOn.h View/Icon/RemoteDesktop.h View/Icon/RemovableMedia.h View/Icon/Warning.h View/Icon/Yes.h View/PixStore.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/PropertyHelper.o: View/PropertyHelper.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/Model.h Model/PatchRecord.h Model/PatchState.h View/NameValueListView.h View/NameValueMenu.h View/PatchListView.h View/PatchMenu.h View/PropertyHelper.h XenServer/Api.h XenServer/XenServer.h 
$(OBJDIR)/View/ResizeDialog.o: View/ResizeDialog.cc View/ResizeDialog.h View/SizeInBytesBox.h 
$(OBJDIR)/View/ResourceHistoryGraph.o: View/ResourceHistoryGraph.cc Base/StringBuffer.h View/ResourceHistoryGraph.h 
$(OBJDIR)/View/ShadowMemoryDialog.o: View/ShadowMemoryDialog.cc View/ShadowMemoryDialog.h 
$(OBJDIR)/View/SizeInBytesBox.o: View/SizeInBytesBox.cc Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h View/SizeInBytesBox.h 
$(OBJDIR)/View/SnapshotMenu.o: View/SnapshotMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Model/Model.h View/SnapshotMenu.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/SnapshotTreeView.o: View/SnapshotTreeView.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h View/PixStore.h View/SnapshotMenu.h View/SnapshotTreeView.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/SnapshotView.o: View/SnapshotView.cc Base/RefObj.h Base/RefPtr.h View/SnapshotMenu.h View/SnapshotTreeView.h View/SnapshotView.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/StorageRepositoryComboBox.o: View/StorageRepositoryComboBox.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h View/StorageRepositoryComboBox.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/StorageRepositoryMenu.o: View/StorageRepositoryMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/NameValueMenu.h View/StorageRepositoryMenu.h XenServer/Api.h XenServer/Constants.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/StorageRepositoryNotebook.o: View/StorageRepositoryNotebook.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h View/NameValueListView.h View/NameValueMenu.h View/Notebook.h View/PropertyHelper.h View/StorageRepositoryMenu.h View/StorageRepositoryNotebook.h View/VirtualDiskImageListView.h View/VirtualDiskImageMenu.h XenServer/Api.h XenServer/Constants.h XenServer/PhysicalBlockDevice.h XenServer/StorageRepository.h XenServer/VirtualDiskImage.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VgaDialog.o: View/VgaDialog.cc View/VgaDialog.h 
$(OBJDIR)/View/View.o: View/View.cc Base/RefObj.h Base/RefPtr.h View/HostMenu.h View/HostTreeView.h View/MainWindow.h View/NameValueMenu.h View/Notebook.h View/NotebookStore.h View/PixStore.h View/StorageRepositoryMenu.h View/View.h View/ViewImpl.h View/VirtualMachineMenu.h View/VirtualMachineStatusWindow.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachineOperationState.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/ViewImpl.o: View/ViewImpl.cc App/Constants.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/MacAddress.h View/AboutDialog.h View/AddCifsDialog.h View/AddVmDialog.h View/AttachCdDialog.h View/AttachHddDialog.h View/AttachNicDialog.h View/BlockDeviceNumberComboBox.h View/CdDeviceComboBox.h View/CdImageListView.h View/ChangeCdDialog.h View/ConnectDialog.h View/CopyVmDialog.h View/CpuDialog.h View/DeleteVmDialog.h View/DeviceNumberComboBox.h View/ExportVmDialog.h View/HardDiskDriveListBox.h View/HardDiskDriveListView.h View/HardDiskDriveSpecDialog.h View/HostMenu.h View/HostTreeView.h View/ImportVmDialog.h View/MainWindow.h View/MemoryDialog.h View/NameDialog.h View/NameValueMenu.h View/NetworkDeviceNumberComboBox.h View/NetworkListView.h View/Notebook.h View/NotebookStore.h View/PixStore.h View/ResizeDialog.h View/ShadowMemoryDialog.h View/SizeInBytesBox.h View/StorageRepositoryComboBox.h View/StorageRepositoryMenu.h View/VgaDialog.h View/View.h View/ViewImpl.h View/VirtualDiskImageListView.h View/VirtualDiskImageMenu.h View/VirtualMachineMenu.h View/VirtualMachineStatusWindow.h View/VirtualMachineTemplateListView.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/PerformanceMonitor.h XenServer/Session.h XenServer/VirtualMachine.h XenServer/VirtualMachineOperationState.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualBlockDeviceMenu.o: View/VirtualBlockDeviceMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/NameValueMenu.h View/VirtualBlockDeviceMenu.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualDiskImageListView.o: View/VirtualDiskImageListView.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Model/ConnectSpec.h Net/MacAddress.h View/NameValueMenu.h View/VirtualDiskImageListView.h View/VirtualDiskImageMenu.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualDiskImageMenu.o: View/VirtualDiskImageMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/NameValueMenu.h View/VirtualDiskImageMenu.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualInterfaceMenu.o: View/VirtualInterfaceMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/NameValueMenu.h View/VirtualInterfaceMenu.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualMachineGenMenu.o: View/VirtualMachineGenMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/NameValueMenu.h View/VirtualMachineGenMenu.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualMachineMemMenu.o: View/VirtualMachineMemMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/NameValueMenu.h View/VirtualMachineMemMenu.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualMachineMenu.o: View/VirtualMachineMenu.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h View/VirtualMachineMenu.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualMachineNotebook.o: View/VirtualMachineNotebook.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Controller/SignalManager.h Exception/Exception.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Net/Console.h Net/MacAddress.h View/ConsoleView.h View/ConsoleViewImpl.h View/ConsoleViewKeyboardInputFilter.h View/ConsoleViewKeyboardInputFilterImpl.h View/CpuGraph.h View/FrameScaler.h View/InputOutputGraph.h View/MemoryGraph.h View/NameValueListView.h View/NameValueMenu.h View/Notebook.h View/PixStore.h View/ResourceHistoryGraph.h View/SnapshotMenu.h View/SnapshotTreeView.h View/SnapshotView.h View/VirtualBlockDeviceMenu.h View/VirtualDiskImageMenu.h View/VirtualInterfaceMenu.h View/VirtualMachineDevicePage.h View/VirtualMachineGenMenu.h View/VirtualMachineMemMenu.h View/VirtualMachineNotebook.h View/VirtualMachinePropertyView.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/PerformanceMonitor.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualMachinePropertyView.o: View/VirtualMachinePropertyView.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/SignalManager.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h View/NameValueListView.h View/NameValueMenu.h View/NetworkMenu.h View/PixStore.h View/PropertyHelper.h View/VirtualBlockDeviceMenu.h View/VirtualDiskImageMenu.h View/VirtualInterfaceMenu.h View/VirtualMachineDevicePage.h View/VirtualMachineGenMenu.h View/VirtualMachineMemMenu.h View/VirtualMachinePropertyView.h XenServer/Api.h XenServer/Constants.h XenServer/Network.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualMachineStatusWindow.o: View/VirtualMachineStatusWindow.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/SignalManager.h File/File.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h View/PixStore.h View/VirtualMachineStatusWindow.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachine.h XenServer/VirtualMachineExporter.h XenServer/VirtualMachineImporter.h XenServer/VirtualMachineOperationState.h XenServer/VirtualMachinePorter.h XenServer/VirtualMachineVerifier.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/View/VirtualMachineTemplateListView.o: View/VirtualMachineTemplateListView.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h View/VirtualMachineTemplateListView.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/CifsSpec.o: XenServer/CifsSpec.cc XenServer/CifsSpec.h 
$(OBJDIR)/XenServer/GetNotificationText.o: XenServer/GetNotificationText.cc Base/RefObj.h Base/RefPtr.h XenServer/Api.h XenServer/Constants.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/GetXenObjectTypeText.o: XenServer/GetXenObjectTypeText.cc Base/RefObj.h Base/RefPtr.h XenServer/Api.h XenServer/Constants.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/HardDiskDriveSpec.o: XenServer/HardDiskDriveSpec.cc Base/RefObj.h Base/RefPtr.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Session.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/Host.o: XenServer/Host.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Model/Model.h Model/PatchBase.h Model/PatchRecord.h Model/PatchState.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/Network.o: XenServer/Network.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Network.h XenServer/Session.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/PerformanceMonitor.o: XenServer/PerformanceMonitor.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/Base64.h Util/ByteString.h XenServer/Api.h XenServer/Constants.h XenServer/PerformanceMonitor.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/PhysicalBlockDevice.o: XenServer/PhysicalBlockDevice.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/PhysicalBlockDevice.h XenServer/Session.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/PhysicalInterface.o: XenServer/PhysicalInterface.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/Session.o: XenServer/Session.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/StorageRepository.o: XenServer/StorageRepository.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h XenServer/Api.h XenServer/Constants.h XenServer/PhysicalBlockDevice.h XenServer/Session.h XenServer/StorageRepository.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualBlockDevice.o: XenServer/VirtualBlockDevice.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualDiskImage.o: XenServer/VirtualDiskImage.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/VirtualInterface.o: XenServer/VirtualInterface.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualMachine.o: XenServer/VirtualMachine.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Macros.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/VirtualMachineArchive.o: XenServer/VirtualMachineArchive.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/File.h File/FileImpl.h Logger/LogLevel.h Logger/Logger.h XenServer/Api.h XenServer/Constants.h XenServer/VirtualMachineArchive.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualMachineExporter.o: XenServer/VirtualMachineExporter.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/File.h File/FileImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualMachine.h XenServer/VirtualMachineArchive.h XenServer/VirtualMachineExporter.h XenServer/VirtualMachineOperationState.h XenServer/VirtualMachinePorter.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualMachineImporter.o: XenServer/VirtualMachineImporter.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/File.h File/FileImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Session.h XenServer/VirtualMachine.h XenServer/VirtualMachineArchive.h XenServer/VirtualMachineImporter.h XenServer/VirtualMachineOperationState.h XenServer/VirtualMachinePorter.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualMachinePorter.o: XenServer/VirtualMachinePorter.cc Base/RefObj.h Base/RefPtr.h File/File.h File/FileImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/UUID.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualMachine.h XenServer/VirtualMachineArchive.h XenServer/VirtualMachineOperationState.h XenServer/VirtualMachinePorter.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/VirtualMachineVerifier.o: XenServer/VirtualMachineVerifier.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h File/File.h File/FileImpl.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/VirtualMachineArchive.h XenServer/VirtualMachineOperationState.h XenServer/VirtualMachinePorter.h XenServer/VirtualMachineVerifier.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/XenEventMonitor.o: XenServer/XenEventMonitor.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Controller/Controller.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Network.h XenServer/PhysicalBlockDevice.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenEventMonitor.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenObjectTypeMap.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/XenObject.o: XenServer/XenObject.cc Base/Atomic.h Base/RefObj.h Base/RefPtr.h Controller/SignalManager.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/XenObjectStore.o: XenServer/XenObjectStore.cc Base/RefObj.h Base/RefPtr.h Controller/Controller.h Model/ConnectSpec.h Net/MacAddress.h XenServer/Api.h XenServer/Constants.h XenServer/Host.h XenServer/Macros.h XenServer/Network.h XenServer/PerformanceMonitor.h XenServer/PhysicalBlockDevice.h XenServer/PhysicalInterface.h XenServer/Session.h XenServer/StorageRepository.h XenServer/VirtualBlockDevice.h XenServer/VirtualDiskImage.h XenServer/VirtualInterface.h XenServer/VirtualMachine.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
$(OBJDIR)/XenServer/XenObjectTypeMap.o: XenServer/XenObjectTypeMap.cc Base/RefObj.h Base/RefPtr.h XenServer/Api.h XenServer/Constants.h XenServer/XenObject.h XenServer/XenObjectTypeMap.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/XenServer.o: XenServer/XenServer.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Util/Base64.h Util/ByteString.h Util/Scrambler.h Util/ScramblerBase.h Util/Util.h XenServer/Api.h XenServer/CifsSpec.h XenServer/Constants.h XenServer/HardDiskDriveSpec.h XenServer/Macros.h XenServer/VirtualMachineSpec.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h 
$(OBJDIR)/XenServer/XenTask.o: XenServer/XenTask.cc Base/RefObj.h Base/RefPtr.h Base/StringBuffer.h Logger/LogLevel.h Logger/Logger.h Logger/Trace.h Model/ConnectSpec.h Net/MacAddress.h Util/Util.h XenServer/Api.h XenServer/Constants.h XenServer/Session.h XenServer/XenObject.h XenServer/XenObjectStore.h XenServer/XenPtr.h XenServer/XenRef.h XenServer/XenServer.h XenServer/XenTask.h 
##### dependency end #####
