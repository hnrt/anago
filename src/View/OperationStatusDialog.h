// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_OPERATIONSTATUSDIALOG_H
#define HNRT_OPERATIONSTATUSDIALOG_H


#include <gtkmm.h>


namespace hnrt
{
    class OperationStatusDialog
        : public Gtk::Dialog
    {
    public:

        OperationStatusDialog(Gtk::Window&, const char*);
        void setStatus(const char*);

    private:

        enum Flag
        {
            OPERATION_IN_PROGRESS = 1,
            OPERATION_COMPLETED = 2,
            OPERATION_FAILED = 4,
            STATUS_CHANGED = 8
        };

        OperationStatusDialog(const OperationStatusDialog&);
        void operator =(const OperationStatusDialog&);
        void onDispatch();

        Gtk::HBox _statusBox;
        Gtk::Image _statusImage;
        Gtk::Label _statusLabel;
        Glib::Dispatcher _dispatcher;
        Glib::Mutex _mutex;
        unsigned int _flags;
        Glib::ustring _status;
    };
}


#endif //!HNRT_OPERATIONSTATUSDIALOG_H
