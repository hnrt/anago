# Anago - XenServer Management Console for Linux

Anago is a graphical user interface for managing Citrix XenServer, working on Intel x86\_64 Linux platform.
Though its functionality is limited as compared with Citrix XenCenter, Anago provides Linux GNOME desktop users with fundamental means for XenServer management.
This means that you can enjoy XenServer only with Linux machines; you don't have to prepare any Microsoft Windows desktop machine just for hosting XenCenter.

* Citrix XenServer is an enterprise-class, cloud-proven, virtualization platform available free of charge at [xenserver.org](https://xenserver.org/).
* Citrix XenCenter is the Windows-native graphical user interface for managing Citrix XenServer. For more information, visit [community blog](https://xenserver.org/partners/developing-products-for-xenserver/21-xencenter-development/88-xc-dev-home.html) at xenserver.org.

## Requirements

* Development has been done on CentOS 6 GNOME desktop with GTKMM 2.4 library.
* libcurl-devel and libxml2-devel package are required to build the executable.
* XenServer SDK is also required to build and to run the executable. It is available free of charge at [xenserver.org](https://xenserver.org/).
