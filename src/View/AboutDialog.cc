// Copyright (C) 2012-2018 Hideaki Narita


#include <libintl.h>
#include "App/Constants.h"
#include "App/License.h"
#include "App/Version.h"
#include "AboutDialog.h"
#include "PixStore.h"


using namespace hnrt;


AboutDialog::AboutDialog()
{
    set_program_name(APPDISPNAME);
    set_copyright(COPYRIGHT);
    set_comments(Glib::ustring::compose(gettext("XenServer Console for Linux\n(built on %1)"), __DATE__));
    set_version(Glib::ustring::compose(gettext("version %1"), VERSION));
    set_icon(PixStore::instance().getApp());
    set_logo(PixStore::instance().getApp());
    set_license(LICENSE);
}


// put this dialog at the center of the main window
void AboutDialog::move(Gtk::Window& parent)
{
    int x = 0, y = 0, cx = 0, cy = 0;
    parent.get_position(x, y);
    parent.get_size(cx, cy);
    int dcx = 0, dcy = 0;
    get_size(dcx, dcy);
    x += (cx - dcx) / 2;
    y += (cy - dcy) / 2;
    Gtk::AboutDialog::move(x, y);
}
