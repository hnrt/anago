// Copyright (C) 2012-2017 Hideaki Narita


#include <gtkmm.h>
#include <libintl.h>
#include <libxml/parser.h>
#include <curl/curl.h>
extern "C" {
#include <xen/api/xen_all.h>
}
#include <stdexcept>
#include "Env/Env.h"
#include "Env/Locale.h"


#define TEXTDOMAIN "anago"


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
        bind_textdomain_codeset(TEXTDOMAIN, "UTF-8");
        textdomain(TEXTDOMAIN);

        //TODO Gtk::Main::run(View::instance());

        Env::fini();

        // cleanup for Xen API
        curl_global_cleanup();
        xen_fini();
        xmlCleanupParser();
    }
    catch (std::runtime_error e)
    {
        g_printerr("Error: %s\n", e.what());
    }
    catch (...)
    {
        g_printerr("Error(%s:%d): Unhandled exception caught.\n", __FILE__, __LINE__);
    }

    return status;
}
