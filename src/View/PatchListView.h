// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_PATCHLISTVIEW_H
#define HNRT_PATCHLISTVIEW_H


#include <time.h>
#include <gtkmm.h>
#include "Base/RefPtr.h"
#include "Model/PatchRecord.h"
#include "Model/PatchState.h"
#include "XenServer/Api.h"
#include "PatchMenu.h"


namespace hnrt
{
    class Host;
    class XenObject;

    class PatchListView
        : public Gtk::TreeView
    {
    public:

        PatchListView(const Host&);
        void clear();
        void set(const PatchRecord&);
        Gtk::ScrolledWindow* createScrolledWindow();

    protected:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colId;
            Gtk::TreeModelColumn<Glib::ustring> colLabel;
            Gtk::TreeModelColumn<Glib::ustring> colDescription;
            Gtk::TreeModelColumn<PatchState> colStatus;
            Gtk::TreeModelColumn<Glib::ustring> colDisplayStatus;
            Gtk::TreeModelColumn<Glib::ustring> colAfterApplyGuidance;
            Gtk::TreeModelColumn<Glib::ustring> colRequired;
            Gtk::TreeModelColumn<Glib::ustring> colConflicting;
            Gtk::TreeModelColumn<time_t> colTimestamp;
            Gtk::TreeModelColumn<Glib::ustring> colDisplayTimestamp;
            Gtk::TreeModelColumn<int64_t> colSize;
            Gtk::TreeModelColumn<Glib::ustring> colDisplaySize;
            Gtk::TreeModelColumn<Glib::ustring> colVersion;
            Gtk::TreeModelColumn<Glib::ustring> colUrl;
            Gtk::TreeModelColumn<Glib::ustring> colPatchUrl;

            Record()
            {
                add(colId);
                add(colLabel);
                add(colDescription);
                add(colStatus);
                add(colDisplayStatus);
                add(colAfterApplyGuidance);
                add(colRequired);
                add(colConflicting);
                add(colTimestamp);
                add(colDisplayTimestamp);
                add(colSize);
                add(colDisplaySize);
                add(colVersion);
                add(colUrl);
                add(colPatchUrl);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        PatchListView(const PatchListView&);
        void operator =(const PatchListView&);
        virtual bool on_button_press_event(GdkEventButton*);
        void onNotify(RefPtr<XenObject>, int);

        const Host& _host;
        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
        PatchMenu _menu;
    };
}


#endif //!HNRT_PATCHLISTVIEW_H
