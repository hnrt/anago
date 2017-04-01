// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEDEVICEPAGE_H
#define HNRT_VIRTUALMACHINEDEVICEPAGE_H


#include <stdexcept>
#include "NameValueListViewSw.h"


namespace hnrt
{
    class NameValueMenu;

    class VirtualMachineDevicePage
        : public Gtk::VPaned
    {
    public:

        VirtualMachineDevicePage(const char* uuid, const char* label1string, const char* label2string)
            : _uuid(uuid)
        {
            _label[0].set_label(label1string);
            _label[1].set_label(label2string);
            _label[0].set_alignment(0, 0.5);
            _label[1].set_alignment(0, 0.5);
            _box[0].pack_start(_label[0], Gtk::PACK_SHRINK);
            _box[0].pack_start(_listViewSw[0]);
            _box[1].pack_start(_label[1], Gtk::PACK_SHRINK);
            _box[1].pack_start(_listViewSw[1]);
            pack1(_box[0], true, true);
            pack2(_box[1], true, true);
            show_all_children();
            show();
            _pMenu[0] = NULL;
            _pMenu[1] = NULL;
            _pMenu[2] = NULL;
        }

        VirtualMachineDevicePage(const char* uuid, const char* label1string, const char* label2string, const char* label3string)
            : _uuid(uuid)
        {
            _label[0].set_label(label1string);
            _label[1].set_label(label2string);
            _label[2].set_label(label3string);
            _label[0].set_alignment(0, 0.5);
            _label[1].set_alignment(0, 0.5);
            _label[2].set_alignment(0, 0.5);
            _box[0].pack_start(_label[0], Gtk::PACK_SHRINK);
            _box[0].pack_start(_listViewSw[0]);
            _box[1].pack_start(_label[1], Gtk::PACK_SHRINK);
            _box[1].pack_start(_listViewSw[1]);
            _box[2].pack_start(_label[2], Gtk::PACK_SHRINK);
            _box[2].pack_start(_listViewSw[2]);
            _subPane[0].pack1(_box[1], true, true);
            _subPane[0].pack2(_box[2], true, true);
            pack1(_box[0], true, true);
            pack2(_subPane[0], true, true);
            show_all_children();
            show();
            _pMenu[0] = NULL;
            _pMenu[1] = NULL;
            _pMenu[2] = NULL;
        }

        virtual ~VirtualMachineDevicePage()
        {
            delete _pMenu[0];
            delete _pMenu[1];
            delete _pMenu[2];
        }

        const Glib::ustring& uuid() const
        {
            return _uuid;
        }

        NameValueListView& listView(int index)
        {
            if (0 <= index && index < 3)
            {
                return _listViewSw[index].listView();
            }
            else
            {
                throw std::runtime_error("VirtualMachineDevicePage::listView: Index out of bounds.");
            }
        }

        void setMenu(int index, NameValueMenu* pMenu)
        {
            if (0 <= index && index < 3)
            {
                delete _pMenu[index];
                _pMenu[index] = pMenu;
                _listViewSw[index].listView().setMenu(pMenu);
            }
            else
            {
                throw std::runtime_error("VirtualMachineDevicePage::menu: Index out of bounds.");
            }
        }

    private:

        VirtualMachineDevicePage(const VirtualMachineDevicePage&);
        void operator =(const VirtualMachineDevicePage&);

        Glib::ustring _uuid;
        Gtk::VBox _box[3];
        Gtk::Label _label[3];
        NameValueListViewSw _listViewSw[3];
        NameValueMenu* _pMenu[3];
        Gtk::VPaned _subPane[1];
    };
}


#endif //!HNRT_VIRTUALMACHINEDEVICEPAGE_H
