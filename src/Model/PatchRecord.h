// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHRECORD_H
#define HNRT_PATCHRECORD_H


#include <time.h>
#include <glibmm/ustring.h>
#include <list>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "PatchState.h"


namespace hnrt
{
    struct PatchRecord
        : public RefObj
    {
        Glib::ustring uuid;
        Glib::ustring label;
        Glib::ustring description;
        Glib::ustring afterApplyGuidance;
        Glib::ustring url;
        Glib::ustring patchUrl;
        Glib::ustring releaseNotes;
        Glib::ustring version;
        time_t timestamp;
        int guidanceMandatory;
        size_t size;
        std::list<Glib::ustring> conflicting;
        std::list<Glib::ustring> required;
        PatchState state;

        inline static RefPtr<PatchRecord> create();

        inline PatchRecord& operator =(const PatchRecord& rhs);

    protected:

        inline PatchRecord();
    };

    inline RefPtr<PatchRecord> PatchRecord::create()
    {
        return RefPtr<PatchRecord>(new PatchRecord());
    }

    inline PatchRecord::PatchRecord()
        : timestamp(0)
        , guidanceMandatory(-1)
        , size(0)
        , state(PatchState::AVAILABLE)
    {
    }

    inline PatchRecord& PatchRecord::operator =(const PatchRecord& rhs)
    {
        uuid = rhs.uuid;
        label = rhs.label;
        description = rhs.description;
        afterApplyGuidance = rhs.afterApplyGuidance;
        url = rhs.url;
        patchUrl = rhs.patchUrl;
        releaseNotes = rhs.releaseNotes;
        version = rhs.version;
        timestamp = rhs.timestamp;
        guidanceMandatory = rhs.guidanceMandatory;
        size = rhs.size;
        conflicting = rhs.conflicting;
        required = rhs.required;
        state = rhs.state;
        return *this;
    }
}


#endif //!HNRT_PATCHRECORD_H
