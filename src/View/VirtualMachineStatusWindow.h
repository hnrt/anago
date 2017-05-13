// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VMSTATUSWINDOW_H
#define HNRT_VMSTATUSWINDOW_H


#include <time.h>
#include <gtkmm.h>
#include <list>
#include "XenServer/VirtualMachineOperationState.h"


namespace hnrt
{
    class XenObject;

    class VirtualMachineStatusWindow
        : public Gtk::Window
    {
    public:

        VirtualMachineStatusWindow(Gtk::Window& parent);
        virtual ~VirtualMachineStatusWindow();
        bool canContinue(const Glib::ustring&);

    private:

        class ListView
            : public Gtk::TreeView
        {
        public:

            ListView();
            void clear();
            void add(RefPtr<XenObject>);
            void update(RefPtr<XenObject>);
            void remove(RefPtr<XenObject>);
            void update();
            VirtualMachineOperationState getState(Gtk::TreeIter);
            Glib::ustring cancel(Gtk::TreeIter);
            void erase(Gtk::TreeIter);

        protected:

            struct Record
                : public Gtk::TreeModel::ColumnRecord
            {
                Gtk::TreeModelColumn<Glib::ustring> colId;
                Gtk::TreeModelColumn<RefPtr<XenObject> > colOperator;
                Gtk::TreeModelColumn<Glib::ustring> colName;
                Gtk::TreeModelColumn<Glib::ustring> colPath;
                Gtk::TreeModelColumn<int64_t> colSize;
                Gtk::TreeModelColumn<VirtualMachineOperationState> colState;
                Gtk::TreeModelColumn<time_t> colStartTime;
                Gtk::TreeModelColumn<Glib::ustring> colDisplaySize;
                Gtk::TreeModelColumn<Glib::ustring> colDisplayState;
                Gtk::TreeModelColumn<Glib::ustring> colElapsedTime;

                Record()
                {
                    add(colId);
                    add(colOperator);
                    add(colName);
                    add(colPath);
                    add(colSize);
                    add(colState);
                    add(colStartTime);
                    add(colDisplaySize);
                    add(colDisplayState);
                    add(colElapsedTime);
                }

            private:

                Record(const Record&);
                void operator =(const Record&);
            };

            ListView(const ListView&);
            void operator =(const ListView&);
            virtual bool on_button_press_event(GdkEventButton*);
            void update(Gtk::TreeModel::Row&);
            void updateSize(Gtk::TreeModel::Row&, int64_t);
            void updateState(Gtk::TreeModel::Row&, VirtualMachineOperationState, int = -1);
            void updateTime(Gtk::TreeModel::Row&, time_t);

            Record _record;
            Glib::RefPtr<Gtk::ListStore> _store;
        };

        VirtualMachineStatusWindow(const VirtualMachineStatusWindow&);
        void operator =(const VirtualMachineStatusWindow&);
        void onObjectCreated(RefPtr<XenObject>, int);
        void onObjectUpdated(RefPtr<XenObject>, int);
        bool onTimedOut();
        void onSelectionChanged();
        void updateButtonSensitivity();
        void onCancel();
        void onDismiss();
        void cancel(const Glib::ustring&);
        void finishCancellation(const Glib::ustring&);

        Gtk::VBox _vbox;
        Gtk::ScrolledWindow _listViewSw;
        ListView _listView;
        Gtk::HButtonBox _buttonBox;
        Gtk::Button _cancelButton;
        Gtk::Button _dismissButton;
        Glib::Mutex _mutex;
        std::list<Glib::ustring> _canceling;
        int _timeoutInUse;
        sigc::connection _timeout;
    };
}


#endif //!HNRT_VMSTATUSWINDOW_H
