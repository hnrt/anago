// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Model/Model.h"
#include "Util/Base64.h"
#include "Util/Scrambler.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "ConnectDialog.h"


using namespace hnrt;


ConnectDialog::ConnectDialog(Gtk::Window& parent, const char* title, const Gtk::StockID& okLabel)
    : Gtk::Dialog(title, parent)
    , _table(4, 2)
    , _dispnameLabel(gettext("Display name:"))
    , _hostnameLabel(gettext("Host name:"))
    , _usernameLabel(gettext("User name:"))
    , _passwordLabel(gettext("Password:"))
    , _editMode(false)
{
    _credStore = Gtk::ListStore::create(_credRecord);
    initStore();

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(okLabel, Gtk::RESPONSE_OK);

    Gtk::VBox* box = get_vbox();
 
    _table.attach(_dispnameLabel, 0, 1, 0, 1);
    _table.attach(_dispnameCombo, 1, 2, 0, 1);
    _table.attach(_hostnameLabel, 0, 1, 1, 2);
    _table.attach(_hostnameEntry, 1, 2, 1, 2);
    _table.attach(_usernameLabel, 0, 1, 2, 3);
    _table.attach(_usernameEntry, 1, 2, 2, 3);
    _table.attach(_passwordLabel, 0, 1, 3, 4);
    _table.attach(_passwordEntry, 1, 2, 3, 4);
    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _dispnameLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _hostnameLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _usernameLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _passwordLabel.set_alignment(1.0, 0.5); // h=right, v=center

    _dispnameCombo.set_model(_credStore);
    _dispnameCombo.set_text_column(_credRecord.colDispname);
    _dispnameCombo.signal_changed().connect(sigc::mem_fun(*this, &ConnectDialog::onDispnameChanged));

    _hostnameEntry.signal_insert_text().connect(sigc::mem_fun(*this, &ConnectDialog::onHostnameChanged1));
    _hostnameEntry.signal_delete_text().connect(sigc::mem_fun(*this, &ConnectDialog::onHostnameChanged2));

    _usernameEntry.signal_insert_text().connect(sigc::mem_fun(*this, &ConnectDialog::onUsernameChanged1));
    _usernameEntry.signal_delete_text().connect(sigc::mem_fun(*this, &ConnectDialog::onUsernameChanged2));

    _passwordEntry.set_visibility(false);
    _passwordEntry.set_invisible_char('*');

    show_all_children();

    validate();
}


void ConnectDialog::initStore()
{
    std::list<RefPtr<Host> > hosts;
    Model::instance().get(hosts);
    for (std::list<RefPtr<Host> >::const_iterator iter = hosts.begin(); iter != hosts.end(); iter++)
    {
        const RefPtr<Host>& host = *iter;
        add(host->getSession().getConnectSpec());
    }
}


void ConnectDialog::add(const ConnectSpec& cs)
{
    Gtk::TreeIter iter = _credStore->get_iter("0"); // point to first item
    Gtk::TreeModel::Row row;
    while (1)
    {
        if (!iter)
        {
            row = *_credStore->append();
            break;
        }
        row = *iter;
        if (row[_credRecord.colLastAccess] < cs.lastAccess)
        {
            row = *_credStore->insert(iter);
            break;
        }
        iter++; // move to next item
    }
    row[_credRecord.colId] = cs.uuid;
    row[_credRecord.colDispname] = cs.displayname;
    row[_credRecord.colHostname] = cs.hostname;
    row[_credRecord.colUsername] = cs.username;
    row[_credRecord.colPassword] = cs.password;
    row[_credRecord.colLastAccess] = cs.lastAccess;
}


void ConnectDialog::validate()
{
    bool ok = true;
    Glib::ustring dispname = _dispnameCombo.get_active_text();
    Glib::ustring hostname = _hostnameEntry.get_text();
    Glib::ustring username = _usernameEntry.get_text();
    if (dispname.empty() || hostname.empty() || username.empty() ||
        dispname.find(",") != Glib::ustring::npos ||
        hostname.find(",") != Glib::ustring::npos ||
        username.find(",") != Glib::ustring::npos)
    {
        ok = false;
    }
    set_response_sensitive(Gtk::RESPONSE_OK, ok);
}


void ConnectDialog::onDispnameChanged()
{
    Gtk::TreeIter iter = _dispnameCombo.get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring uuid = row[_credRecord.colId];
        Glib::ustring hostname = row[_credRecord.colHostname];
        Glib::ustring username = row[_credRecord.colUsername];
        Glib::ustring password = row[_credRecord.colPassword];
        if (_editMode)
        {
            _uuid = uuid;
        }
        _hostnameEntry.set_text(hostname);
        _usernameEntry.set_text(username);
        Base64Decoder d1(password.c_str());
        Descrambler d2(d1.getValue(), d1.getLength());
        _passwordEntry.set_text(Glib::ustring(reinterpret_cast<const char*>(d2.getValue())));
    }
    validate();
}


void ConnectDialog::onHostnameChanged1(const Glib::ustring& text, int* position)
{
    validate();
}


void ConnectDialog::onHostnameChanged2(int start, int end)
{
    validate();
}


void ConnectDialog::onUsernameChanged1(const Glib::ustring& text, int* position)
{
    validate();
}


void ConnectDialog::onUsernameChanged2(int start, int end)
{
    validate();
}


void ConnectDialog::selectFirstHostname()
{
    Gtk::TreeIter iter = _credStore->get_iter("0"); // point to first item
    if (iter)
    {
        _dispnameCombo.set_active(iter);
    }
    else
    {
        _dispnameCombo.unset_active();
    }
}


void ConnectDialog::select(const ConnectSpec& cs)
{
    Gtk::TreeIter iter = _credStore->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring id = row[_credRecord.colId];
        if (cs.uuid == id)
        {
            _editMode = true;
            _dispnameCombo.set_active(iter);
            return;
        }
        iter++; // move to next item
    }
    _dispnameCombo.unset_active();
    _usernameEntry.set_text("");
    _passwordEntry.set_text("");
}


ConnectSpec ConnectDialog::getConnectSpec() const
{
    ConnectSpec cs;
    if (_uuid.bytes())
    {
        cs.uuid = _uuid;
    }
    cs.displayname = getDispname();
    cs.hostname = getHostname();
    cs.username = getUsername();
    cs.password = getPassword();
    return cs;
}


Glib::ustring ConnectDialog::getDispname() const
{
    return _dispnameCombo.get_active_text();
}


Glib::ustring ConnectDialog::getHostname() const
{
    return _hostnameEntry.get_text();
}


Glib::ustring ConnectDialog::getUsername() const
{
    return _usernameEntry.get_text();
}


Glib::ustring ConnectDialog::getPassword() const
{
    Glib::ustring password = _passwordEntry.get_text();
    Scrambler e1(password.data(), password.bytes());
    Base64Encoder e2(e1.getValue(), e1.getLength());
    return Glib::ustring(e2.getValue());
}
