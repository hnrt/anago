// Copyright (C) 2017 Hideaki Narita


#include <libintl.h>
#include "Controller/SignalManager.h"
#include "Model/PatchRecord.h"
#include "XenServer/Patch.h"
#include "OperationStatusDialog.h"


using namespace hnrt;


OperationStatusDialog::OperationStatusDialog(Gtk::Window& parent, Patch& patch)
    : Gtk::Dialog(gettext("Patch"), parent)
{
    set_default_size(-1, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);

    Gtk::VBox* box = get_vbox();

    _statusBox.pack_start(_statusImage, Gtk::PACK_SHRINK);
    _statusBox.pack_start(_statusLabel, Gtk::PACK_EXPAND_WIDGET);
    box->pack_start(_statusBox, Gtk::PACK_EXPAND_WIDGET);

    show_all_children();

    SignalManager::instance().xenObjectSignal(patch).connect(sigc::mem_fun(*this, &OperationStatusDialog::onUpdated));
}


void OperationStatusDialog::onUpdated(RefPtr<XenObject> object, int what)
{
    RefPtr<Patch> patch = RefPtr<Patch>::castStatic(object);
    switch (what)
    {
    case XenObject::DESTROYED:
        break;
    case XenObject::PATCH_UPLOAD_FILE_ERROR:
        set_title(
            Glib::ustring::compose(gettext("Upload patch %1"),
                                   patch->getRecord()->label));
        _statusLabel.set_text(gettext("File not available."));
        break;
    case XenObject::PATCH_UPLOAD_PENDING:
        set_title(
            Glib::ustring::compose(gettext("Upload patch %1"),
                                   patch->getRecord()->label));
        _statusLabel.set_text(
            Glib::ustring::compose(gettext("Uploading...\n\n%1"),
                                   patch->getPath()));
        break;
    case XenObject::PATCH_UPLOAD_PRINT:
    case XenObject::PATCH_APPLY_PRINT:
        _statusLabel.set_text(
            Glib::ustring::compose(gettext("%1\n\n%2"),
                                   _statusLabel.get_text(),
                                   patch->getOutput()));
        break;
    case XenObject::PATCH_UPLOAD_PRINT_ERROR:
    case XenObject::PATCH_APPLY_PRINT_ERROR:
        _statusLabel.set_text(
            Glib::ustring::compose(gettext("%1\n\n%2"),
                                   _statusLabel.get_text(),
                                   patch->getErrorOutput()));
        break;
    case XenObject::PATCH_UPLOAD_EXIT:
    case XenObject::PATCH_APPLY_EXIT:
        _statusLabel.set_text(
            Glib::ustring::compose(gettext("%1\n\nExit code %2"),
                                   _statusLabel.get_text(),
                                   patch->getExitCode()));
        break;
    case XenObject::PATCH_APPLY_PENDING:
        set_title(
            Glib::ustring::compose(gettext("Apply patch %1"),
                                   patch->getRecord()->label));
        _statusLabel.set_text(
            Glib::ustring::compose(gettext("Applying...\n\n%1"),
                                   patch->getRecord()->label));
        break;
    default:
        break;
    }
}
