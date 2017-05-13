// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_POFILE_H
#define HNRT_POFILE_H


#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>


namespace hnrt
{
    class MsgItem;

    class StringComparator
    {
    public:

        bool operator ()(const char* a, const char* b) const
        {
            return strcmp(a, b) < 0;
        }
    };

    typedef std::map<const char*, MsgItem*, StringComparator> MsgMap;
    typedef std::pair<const char*, MsgItem*> MsgEntry;

    class PoFile
    {
    public:

        PoFile(const char*);
        ~PoFile();
        bool open(FILE* = stderr);
        void close();
        void merge(const PoFile&);
        bool updateWithTemplate(const PoFile&);
        bool translateWithId();
        void printUntranslated();
        void replaceLine(size_t, const char*);
        int lines() const { return static_cast<int>(_text.size()); }
        const char* operator[] (size_t index) const { return _text[index]; }
        void print(FILE*) const;

    private:

        void pushBack(const char*);
        void print(FILE*, const MsgItem&, bool = false) const;
        void print(FILE*, const MsgItem&, const PoFile& other, const MsgItem&) const;
        void createFileNamesForUpdate(std::string&, std::string&);
        bool updateFile(const std::string&, const std::string&);

        std::string _path;
        std::vector<char*> _text;
        std::vector<MsgItem*> _items;
        MsgMap _msgidMap;
    };
}


#endif //!HNRT_POFILE_H
