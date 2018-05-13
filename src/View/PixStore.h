// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_PIXSTORE_H
#define HNRT_PIXSTORE_H


#include <gtkmm.h>


namespace hnrt
{
    class XenObject;

    class PixStore
    {
    public:

        static void init();
        static void fini();
        static PixStore& instance();

        virtual ~PixStore() {}
        Glib::RefPtr<Gdk::Pixbuf> getApp() const { return _pixApp; }
        Glib::RefPtr<Gdk::Pixbuf> getYes() const { return _pixYes; }
        Glib::RefPtr<Gdk::Pixbuf> getNo() const { return _pixNo; }
        Glib::RefPtr<Gdk::Pixbuf> getComputer() const { return _pixComputer; }
        Glib::RefPtr<Gdk::Pixbuf> getMemory() const { return _pixMemory; }
        Glib::RefPtr<Gdk::Pixbuf> get(const XenObject&) const;

    protected:

        PixStore();
        PixStore(const PixStore&);
        void operator =(const PixStore&);

        Glib::RefPtr<Gdk::Pixbuf> _pixApp;
        Glib::RefPtr<Gdk::Pixbuf> _pixCdRom;
        Glib::RefPtr<Gdk::Pixbuf> _pixComputer;
        Glib::RefPtr<Gdk::Pixbuf> _pixError;
        Glib::RefPtr<Gdk::Pixbuf> _pixHardDisk;
        Glib::RefPtr<Gdk::Pixbuf> _pixHourglass;
        Glib::RefPtr<Gdk::Pixbuf> _pixMemory;
        Glib::RefPtr<Gdk::Pixbuf> _pixNetworkAdapter;
        Glib::RefPtr<Gdk::Pixbuf> _pixNo;
        Glib::RefPtr<Gdk::Pixbuf> _pixPause;
        Glib::RefPtr<Gdk::Pixbuf> _pixPowerOff;
        Glib::RefPtr<Gdk::Pixbuf> _pixPowerOn;
        Glib::RefPtr<Gdk::Pixbuf> _pixRemovableMedia;
        Glib::RefPtr<Gdk::Pixbuf> _pixWarning;
        Glib::RefPtr<Gdk::Pixbuf> _pixYes;
    };
}


#endif //!HNRT_PIXSTORE_H
