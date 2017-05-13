// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_STORAGEREPOSITORYNOTEBOOK_H
#define HNRT_STORAGEREPOSITORYNOTEBOOK_H


#include "Base/RefPtr.h"
#include "NameValueListView.h"
#include "Notebook.h"
#include "StorageRepositoryMenu.h"
#include "VirtualDiskImageListView.h"


namespace hnrt
{
    class StorageRepository;
    class XenObject;

    class StorageRepositoryNotebook
        : public Notebook
    {
    public:

        static RefPtr<Notebook> create(const RefPtr<StorageRepository>&);

        ~StorageRepositoryNotebook();

    private:

        StorageRepositoryNotebook(const RefPtr<StorageRepository>&);
        StorageRepositoryNotebook(const StorageRepositoryNotebook&);
        void operator =(const StorageRepositoryNotebook&);
        void onSrUpdated(RefPtr<XenObject> object, int what);

        Gtk::VPaned _genBox;

        Gtk::VBox _srBox;
        Gtk::Label _srLabel;
        NameValueListView _srLv;

        StorageRepositoryMenu _srMenu;

        Gtk::VBox _pbdBox;
        Gtk::Label _pbdLabel;
        NameValueListView _pbdLv;

        Gtk::VBox _vdiBox;
        VirtualDiskImageListView _vdiLv;

        RefPtr<StorageRepository> _sr;

        sigc::connection _connection;
    };
}


#endif //!HNRT_STORAGEREPOSITORYNOTEBOOK_H
