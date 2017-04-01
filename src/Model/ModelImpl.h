// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODELIMPL_H
#define HNRT_MODELIMPL_H


#include <glibmm.h>
#include <map>
#include "Logger/Logger.h"
#include "Model.h"


namespace hnrt
{
    class Json;
    class Host;

    class ModelImpl
        : public Model
    {
    public:

        ModelImpl();
        ~ModelImpl();
        void init();
        void fini();
        virtual void load();
        virtual void save();
        virtual void clear();
        virtual int get(std::list<RefPtr<Host> >&);
        virtual void add(const ConnectSpec&);
        virtual void remove(Session&);
        virtual void removeAllSessions();

        virtual void deselectAll();
        virtual void select(const RefPtr<XenObject>&);
        virtual int getSelected(std::list<RefPtr<Host> >&);
        virtual int getSelected(std::list<RefPtr<VirtualMachine> >&);
        virtual int getSelected(std::list<RefPtr<StorageRepository> >&);
        virtual RefPtr<Host> getSelectedHost();

        virtual void selectSnapshot(const RefPtr<VirtualMachine>&);
        virtual void deselectSnapshot();

        virtual RefPtr<PatchBase> getPatchBase();

        virtual const char* getAppDir() const { return _appDir.c_str(); }

        virtual int getWidth();
        virtual void setWidth(int);
        virtual int getHeight();
        virtual void setHeight(int);
        virtual int getPane1Width();
        virtual void setPane1Width(int);
        virtual bool getConsoleEnabled(const Glib::ustring&);
        virtual void setConsoleEnabled(const Glib::ustring&, bool);
        virtual bool getConsoleScale(const Glib::ustring&);
        virtual void setConsoleScale(const Glib::ustring&, bool);

    private:

        struct ConsoleInfo
        {
            Glib::ustring uuid;
            bool enabled;
            bool scale;

            ConsoleInfo(const Glib::ustring uuid_)
                : uuid(uuid_)
                , enabled(true)
                , scale(false)
            {
            }

            ConsoleInfo(const ConsoleInfo& src)
            {
                assign(src);
            }

            void operator =(const ConsoleInfo& rhs)
            {
                assign(rhs);
            }

            void assign(const ConsoleInfo& rhs)
            {
                uuid = rhs.uuid;
                enabled = rhs.enabled;
                scale = rhs.scale;
            }
        };

        typedef std::map<Glib::ustring, ConsoleInfo> ConsoleMap;
        typedef std::pair<Glib::ustring, ConsoleInfo> ConsoleEntry;

        ModelImpl(const ModelImpl&);
        void operator =(const ModelImpl&);
        void loadV1(const Json&);
        ConsoleInfo& getConsoleInfo(const Glib::ustring&);

        Glib::ustring _path;
        Glib::ustring _appDir;
        Glib::RecMutex _mutex;
        std::list<RefPtr<Host> > _hosts;
        std::list<RefPtr<XenObject> > _selected;
        RefPtr<VirtualMachine> _selectedSnapshot;
        RefPtr<PatchBase> _patchBase;

        int _width;
        int _height;
        int _pane1Width;
        ConsoleMap _consoleMap;
    };
}


#endif //!HNRT_MODELIMPL_H
