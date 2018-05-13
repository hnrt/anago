// Copyright (C) 2012-2018 Hideaki Narita


#include "FileImpl.h"


using namespace hnrt;


RefPtr<File> File::create(const char* path, const char* mode)
{
    return FileImpl::create(path, mode);
}


File::File()
{
}


File::~File()
{
}
