// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MAINWINDOW_H
#define HNRT_MAINWINDOW_H


#include <gtkmm.h>
#include "Base/RefPtr.h"
#include "Notebook.h"
#include "NotebookStore.h"
#include "HostTreeView.h"


namespace hnrt
{
    class PerformanceMonitor;
    class XenObject;

    class MainWindow
        : public Gtk::Window
    {
    public:

        MainWindow();
        virtual ~MainWindow();
        int getWidth() const { return _width; }
        int getHeight() const { return _height; }
        void setSize(int cx, int cy);
        int getPane1Width() const;
        void setPane1Width(int);
        void clear();
        bool addObject(RefPtr<XenObject>&);
        void removeObject(RefPtr<XenObject>&);
        void updateObject(RefPtr<XenObject>&, int);
        void addPerformanceMonitor(RefPtr<PerformanceMonitor>&);

    private:

        MainWindow(const MainWindow&);
        void operator =(const MainWindow&);
        void initStockItems();
        bool onClose(GdkEventAny*);
        bool onWindowStateChange(GdkEventWindowState*);
        void onResize(Gtk::Allocation&);
        void onHostTreeViewSelectionChanged();
        void addNotebook(Glib::RefPtr<Notebook>&);
        void removeNotebook(Glib::RefPtr<Notebook>&);
        void showNotebook(const Glib::RefPtr<Notebook>&);
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
        HostTreeView _serverTreeView;
        Gtk::HBox _box2;
        Glib::RefPtr<Notebook> _defaultNotebook;
        Glib::RefPtr<Notebook> _currentNotebook;
        NotebookStore _notebookStore;

        int _width;
        int _height;
        GdkWindowState _windowState;
    };
}


#endif //!HNRT_MAINWINDOW_H
