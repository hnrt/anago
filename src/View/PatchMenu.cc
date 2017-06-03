// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Controller/Controller.h"
#include "Model/Model.h"
#include "XenServer/Host.h"
#include "PatchMenu.h"


using namespace hnrt;


PatchMenu::PatchMenu()
    : _menuBrowse(gettext("Browse web page"))
    , _menuDownload(gettext("Download from web site"))
    , _menuUpload(gettext("Upload to pool"))
    , _menuApply(gettext("Apply to host"))
    , _menuClean(gettext("Clean from pool"))
    , _menuCancel(gettext("Cancel"))
{
    append(_menuBrowse);
    append(_menuDownload);
    append(_menuUpload);
    append(_menuApply);
    append(_menuClean);
    append(*Gtk::manage(new Gtk::SeparatorMenuItem));
    append(_menuCancel);
    show_all_children();
    signal_deactivate().connect(sigc::mem_fun(*this, &PatchMenu::onDeactivate));
    signal_selection_done().connect(sigc::mem_fun(*this, &PatchMenu::onSelectionDone));
    _menuBrowse.signal_activate().connect(sigc::mem_fun(*this, &PatchMenu::onBrowse));
    _menuDownload.signal_activate().connect(sigc::mem_fun(*this, &PatchMenu::onDownload));
    _menuUpload.signal_activate().connect(sigc::mem_fun(*this, &PatchMenu::onUpload));
    _menuApply.signal_activate().connect(sigc::mem_fun(*this, &PatchMenu::onApply));
    _menuClean.signal_activate().connect(sigc::mem_fun(*this, &PatchMenu::onClean));
}


PatchMenu::~PatchMenu()
{
}


void PatchMenu::popup(guint button, guint32 activate_time, const char* uuid, PatchState state)
{
    _uuid = uuid;
    RefPtr<Host> host = Model::instance().getSelectedHost();
    bool isHostBusy = host->isBusy();
    _menuBrowse.set_sensitive(true);
    _menuDownload.set_sensitive(state == PatchState::AVAILABLE && !isHostBusy ? true : false);
    _menuUpload.set_sensitive(state == PatchState::DOWNLOADED && !isHostBusy ? true : false);
    _menuApply.set_sensitive(state == PatchState::UPLOADED && !isHostBusy ? true : false);
    _menuClean.set_sensitive((state == PatchState::UPLOADED || state == PatchState::APPLIED || state == PatchState::APPLY_FAILURE) && !isHostBusy ? true : false);
    Gtk::Menu::popup(button, activate_time);
}


void PatchMenu::onDeactivate()
{
    // This is called right after the closure of the popup window.
}


void PatchMenu::onSelectionDone()
{
    // This is called after all the things are done.
}


void PatchMenu::onBrowse()
{
    Controller::instance().browsePatchPage(_uuid);
}


void PatchMenu::onDownload()
{
    Controller::instance().downloadPatch(_uuid);
}


void PatchMenu::onUpload()
{
    Controller::instance().uploadPatch(_uuid);
}


void PatchMenu::onApply()
{
    Controller::instance().applyPatch(_uuid);
}


void PatchMenu::onClean()
{
    Controller::instance().cleanPatch(_uuid);
}
