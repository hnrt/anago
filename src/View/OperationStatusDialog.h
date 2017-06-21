// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_OPERATIONSTATUSDIALOG_H
#define HNRT_OPERATIONSTATUSDIALOG_H


#include <gtkmm.h>
#include "Base/RefPtr.h"


namespace hnrt
{
    class Patch;
    class XenObject;

    class OperationStatusDialog
        : public Gtk::Dialog
    {
    public:

        OperationStatusDialog(Gtk::Window&, Patch&);

    private:

        OperationStatusDialog(const OperationStatusDialog&);
        void operator =(const OperationStatusDialog&);
        void onUpdated(RefPtr<XenObject>, int);

        Gtk::HBox _statusBox;
        Gtk::Image _statusImage;
        Gtk::Label _statusLabel;
    };
}


#endif //!HNRT_OPERATIONSTATUSDIALOG_H
