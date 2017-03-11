// Copyright (C) 2012-2017 Hideaki Narita


#include <gtkmm.h>
#include <libintl.h>
#include <libxml/parser.h>
#include <curl/curl.h>
extern "C" {
#include <xen/api/xen_all.h>
}
#include <stdexcept>
#include "Controller/Controller.h"
#include "Env/Env.h"
#include "Env/Locale.h"
#include "View/View.h"
#include "Constants.h"


using namespace hnrt;


int main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;

    try
    {
        Gtk::Main kit(argc, argv);
        Glib::thread_init();

        // initialization for Xen API
        xmlInitParser();
        xmlKeepBlanksDefault(0);
        xen_init();
        curl_global_init(CURL_GLOBAL_ALL);

        Env::init();

        // initialization for UI localization
        bindtextdomain(TEXTDOMAIN, Locale::instance().getMessageCatalogDir(TEXTDOMAIN).c_str());
        bind_textdomain_codeset(TEXTDOMAIN, CODESET);
        textdomain(TEXTDOMAIN);

        Controller::init();
        View::init();

        Gtk::Main::run(View::instance().getWindow());

        View::fini();
        Controller::fini();
        Env::fini();

        // cleanup for Xen API
        curl_global_cleanup();
        xen_fini();
        xmlCleanupParser();
    }
    catch (std::runtime_error e)
    {
        g_printerr("Error: %s\n", e.what());
        status = EXIT_FAILURE;
    }
    catch (...)
    {
        g_printerr("Error: Unhandled exception caught.\n");
        status = EXIT_FAILURE;
    }

    return status;
}
