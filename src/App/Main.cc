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
#include "Net/PingAgent.h"
#include "Thread/ThreadManager.h"
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

        ThreadManager::init();
        Logger::init();
        Env::init();
        PingAgent::init();

        // initialization for UI localization
        bindtextdomain(TEXTDOMAIN, Locale::instance().getMessageCatalogDir(TEXTDOMAIN).c_str());
        bind_textdomain_codeset(TEXTDOMAIN, CODESET);
        textdomain(TEXTDOMAIN);

        Model::init();
        Controller::init();
        View::init();

        try
        {
            Model::instance().load();
            Controller::instance().parseCommandLine(argc, argv);
            View::instance().load();
            Gtk::Main::run(View::instance().getWindow());
            View::instance().save();
            Model::instance().save();
        }
        catch (Glib::ustring msg)
        {
            Logger::instance().error(msg.c_str());
            View::instance().showError(msg);
            status = EXIT_FAILURE;
        }
        catch (std::bad_alloc e)
        {
            Glib::ustring msg("Out of memory.");
            Logger::instance().error(msg.c_str());
            View::instance().showError(msg);
            status = EXIT_FAILURE;
        }
        catch (std::runtime_error e)
        {
            Glib::ustring msg(e.what());
            Logger::instance().error(msg.c_str());
            View::instance().showError(msg);
            status = EXIT_FAILURE;
        }

        View::instance().clear();
        Model::instance().clear();

        View::fini();
        Controller::fini();
        Model::fini();

        PingAgent::fini();
        Env::fini();
        Logger::fini();
        ThreadManager::fini();

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
