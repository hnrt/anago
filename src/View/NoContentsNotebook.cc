// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "NoContentsNotebook.h"
#include "Notebook.h"


using namespace hnrt;


RefPtr<Notebook> NoContentsNotebook::create(const char* text)
{
    return RefPtr<Notebook>(new NoContentsNotebook(text));
}


NoContentsNotebook::NoContentsNotebook(const char* text)
{
    _label.set_text(text);
    _box.pack_start(_label);

    append_page(_box, gettext("Information"));

    show_all_children();
}
