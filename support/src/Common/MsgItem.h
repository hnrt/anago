// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MSGITEM_H
#define HNRT_MSGITEM_H


#include <string>
#include <vector>


namespace hnrt
{
    class PoFile;

    class MsgItem
    {
    public:

        MsgItem();
        MsgItem(const MsgItem&);
        ~MsgItem();
        int headerLine() const { return _headerLine; }
        int headerCount() const { return _headerCount; }
        const char* location(int i) const { return _locations[i].c_str(); }
        int locationCount() const { return static_cast<int>(_locations.size()); }
        int msgidLine() const { return _msgidLine; }
        int msgidCount() const { return _msgidCount; }
        const char* msgid() const { return _msgid.c_str(); }
        int msgid2Line() const { return _msgid2Line; }
        int msgid2Count() const { return _msgid2Count; }
        const char* msgid2() const { return _msgid2.c_str(); }
        int msgstrLine() const { return _msgstrLine; }
        int msgstrCount() const { return _msgstrCount; }
        const char* msgstr() const { return _msgstr.c_str(); }
        int msgstr2Line() const { return _msgstr2Line; }
        int msgstr2Count() const { return _msgstr2Count; }
        const char* msgstr2() const { return _msgstr2.c_str(); }
        void clear();
        void setHeaderLine(int value) { _headerLine = value; _headerCount = 1; }
        void incHeaderLine() { _headerCount++; }
        void addLocation(const char*);
        void addLocation(const char*, size_t);
        void setMsgidLine(int value) { _msgidLine = value; _msgidCount = 1; }
        void incMsgidLine() { _msgidCount++; }
        void addMsgid(const char*, size_t);
        void setMsgid2Line(int value) { _msgid2Line = value; _msgid2Count = 1; }
        void incMsgid2Line() { _msgid2Count++; }
        void addMsgid2(const char*, size_t);
        void setMsgstrLine(int value) { _msgstrLine = value; _msgstrCount = 1; }
        void incMsgstrLine() { _msgstrCount++; }
        void addMsgstr(const char*, size_t);
        void setMsgstr2Line(int value) { _msgstr2Line = value; _msgstr2Count = 1; }
        void incMsgstr2Line() { _msgstr2Count++; }
        void addMsgstr2(const char*, size_t);
        bool updateRevisionDate(PoFile&);

    private:

        int _headerLine;
        int _headerCount;
        std::vector<std::string> _locations;
        int _msgidLine;
        int _msgidCount;
        std::string _msgid;
        int _msgid2Line;
        int _msgid2Count;
        std::string _msgid2;
        int _msgstrLine;
        int _msgstrCount;
        std::string _msgstr;
        int _msgstr2Line;
        int _msgstr2Count;
        std::string _msgstr2;
    };
}


#endif //!HNRT_MSGITEM_H
