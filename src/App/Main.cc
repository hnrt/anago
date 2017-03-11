// Copyright (C) 2012-2017 Hideaki Narita


#include <gtkmm.h>
#include <libintl.h>
#include <libxml/parser.h>
#include <curl/curl.h>
#include <stdexcept>
#include "Controller/Controller.h"
#include "Env/Env.h"
#include "Env/Locale.h"
#include "Logger/Logger.h"
#include "Model/Model.h"
#include "View/View.h"
#include "XenServer/Api.h"
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

        Logger::init();
        Env::init();

        // initialization for UI localization
        bindtextdomain(TEXTDOMAIN, Locale::instance().getMessageCatalogDir(TEXTDOMAIN).c_str());
        bind_textdomain_codeset(TEXTDOMAIN, CODESET);
        textdomain(TEXTDOMAIN);

        Model::init();
        Controller::init();
        View::init();

        try
        {
            Controller::instance().parseCommandLine(argc, argv);
            Gtk::Main::run(View::instance().getWindow());
        }
        catch (std::bad_alloc e)
        {
            Logger::instance().error("Out of memory.");
            status = EXIT_FAILURE;
        }
        catch (std::runtime_error e)
        {
            Logger::instance().error("%s", e.what());
            status = EXIT_FAILURE;
        }

        View::fini();
        Controller::fini();
        Model::fini();
        Env::fini();
        Logger::fini();

        // cleanup for Xen API
        curl_global_cleanup();
        xen_fini();
        xmlCleanupParser();
    }
    catch (std::bad_alloc e)
    {
        g_printerr("ERROR: Out of memory.\n");
        status = EXIT_FAILURE;
    }
    catch (std::runtime_error e)
    {
        g_printerr("ERROR: %s\n", e.what());
        status = EXIT_FAILURE;
    }
    catch (...)
    {
        g_printerr("ERROR: Unhandled exception caught.\n");
        status = EXIT_FAILURE;
    }

    return status;
}
