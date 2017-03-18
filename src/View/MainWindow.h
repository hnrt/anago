// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MAINWINDOW_H
#define HNRT_MAINWINDOW_H


#include <gtkmm.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "ServerTreeView.h"


namespace hnrt
{
    class MainWindow
        : public Gtk::Window
    {
    public:

        MainWindow();
        ~MainWindow();
        void setPane1Width(int);
        void clear();

    private:

        MainWindow(const MainWindow&);
        void operator =(const MainWindow&);
        void initStockItems();
        bool onClose(GdkEventAny*);
        bool onWindowStateChange(GdkEventWindowState*);
        void onResize(Gtk::Allocation&);
        void onObjectCreated(RefPtr<RefObj>, int);
        void onObjectUpdated(RefPtr<RefObj>, int);
        void removeObject(RefPtr<RefObj>&);
        void updateObject(RefPtr<RefObj>&, int);
        void onServerTreeViewSelectionChanged();
        void updateSensitivity();

        Glib::RefPtr<Gtk::IconFactory> _iconFactory;
        Gtk::StockID _stockIdAddHost;
        Gtk::StockID _stockIdPowerOn;
        Gtk::StockID _stockIdPowerOff;
        Gtk::StockID _stockIdAddVm;
        Gtk::StockID _stockIdStartVm;
        Gtk::StockID _stockIdShutdownVm;
        Gtk::StockID _stockIdRebootVm;
        Gtk::StockID _stockIdSuspendVm;
        Gtk::StockID _stockIdResumeVm;
        Gtk::StockID _stockIdChangeCd;
        Gtk::StockID _stockIdAuth;

        Glib::RefPtr<Gtk::UIManager> _uiManager;
        Gtk::VBox _box;
        Gtk::HPaned _hpaned;
        Gtk::ScrolledWindow _sw1;
        ServerTreeView _serverTreeView;
        Gtk::HBox _box2;

        GdkWindowState _windowState;
    };
}


#endif //!HNRT_MAINWINDOW_H
