// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIEW_H
#define HNRT_VIEW_H


#include <gtkmm.h>
#include <list>


namespace hnrt
{
    struct ConnectSpec;

    class View
    {
    public:

        static void init();
        static void fini();
        static View& instance();
        static void update();

        virtual const Glib::ustring& getDisplayName() = 0;
        virtual Gtk::Window& getWindow() = 0;
        virtual void load() = 0;
        virtual void save() = 0;
        virtual void clear() = 0;
        virtual void about() = 0;
        virtual void showInfo(const Glib::ustring&) = 0;
        virtual void showWarning(const Glib::ustring&) = 0;
        virtual void showError(const Glib::ustring&) = 0;
        virtual bool getConnectSpec(ConnectSpec&) = 0;
        virtual bool confirmServerToRemove(const char*) = 0;
        virtual void showBusyServers(const std::list<Glib::ustring>&) = 0;
        virtual bool confirmServersToShutdown(const std::list<Glib::ustring>&, bool) = 0;
        virtual bool getName(const char*, Glib::ustring&, Glib::ustring&) = 0;
        virtual bool getCpuSettings(int64_t&, int64_t&, int&) = 0;
        virtual bool getMemorySettings(int64_t&, int64_t&, int64_t&, int64_t&) = 0;
        virtual bool getShadowMemorySettings(double&) = 0;
    };
}


#endif //!HNRT_VIEW_H
