// Copyright (C) 2012-2017 Hideaki Narita


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "PoFile.h"
#include "MsgItem.h"


using namespace hnrt;


PoFile::PoFile(const char* path)
    : _path(path)
{
}


PoFile::~PoFile()
{
    close();
}


bool PoFile::open(FILE* log)
{
    close();

    FILE* fp = fopen(_path.c_str(), "r");
    if (!fp)
    {
        char* msg = strerror(errno);
        fprintf(stderr, "%s: Error: Unable to open for reading: %s\n", _path.c_str(), msg);
        return false;
    }

    const size_t bufsz = 262144;
    char* buf = new char[bufsz];

    while (fgets(buf, bufsz, fp))
    {
        pushBack(buf);
    }

    delete[] buf;

    fclose(fp);

    MsgItem* item = new MsgItem;

    size_t i = 0;
    while (i < _text.size())
    {
        char* p1 = _text[i];

        if (!strncmp(p1, "#: ", 3))
        {
            item->setHeaderLine(static_cast<int>(i));
            item->addLocation(p1);
            if (++i == _text.size())
            {
                fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
                goto done2;
            }
            p1 = _text[i];
            while (!strncmp(p1, "#: ", 3))
            {
                item->incHeaderLine();
                item->addLocation(p1);
                if (++i == _text.size())
                {
                    fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
                    goto done2;
                }
                p1 = _text[i];
            }
        }
        else if (p1[0] == '#')
        {
            item->setHeaderLine(static_cast<int>(i));
            if (++i == _text.size())
            {
                fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
                goto done2;
            }
            p1 = _text[i];
        }
        else
        {
            i++;
            continue;
        }

        while (p1[0] == '#')
        {
            item->incHeaderLine();
            if (++i == _text.size())
            {
                fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
                goto done2;
            }
            p1 = _text[i];
        }

        if (strncmp(p1, "msgid \"", 7))
        {
            fprintf(stderr, "%s(%zu): Error: Missing msgid.\n", _path.c_str(), i + 1);
            goto done2;
        }
        char* p2 = p1 + 7;
        char* p3 = strrchr(p2, '\"');
        if (!p3)
        {
            fprintf(stderr, "%s(%zu): Error: Malformed msgid.\n", _path.c_str(), i + 1);
            goto done2;
        }
        item->setMsgidLine(static_cast<int>(i));
        item->addMsgid(p2, p3 - p2);
        if (++i == _text.size())
        {
            fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
            goto done2;
        }
        p1 = _text[i];

        while (p1[0] == '\"')
        {
            p2 = p1 + 1;
            p3 = strrchr(p2, '\"');
            if (!p3)
            {
                fprintf(stderr, "%s(%zu): Error: Malformed quoted string.\n", _path.c_str(), i + 1);
                goto done2;
            }
            item->incMsgidLine();
            item->addMsgid(p2, p3 - p2);
            if (++i == _text.size())
            {
                fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
                goto done2;
            }
            p1 = _text[i];
        }

        if (!strncmp(p1, "msgid_plural \"", 14))
        {
            p2 = p1 + 14;
            p3 = strrchr(p2, '\"');
            if (!p3)
            {
                fprintf(stderr, "%s(%zu): Error: Malformed msgid_plural.\n", _path.c_str(), i + 1);
                goto done2;
            }
            item->setMsgid2Line(static_cast<int>(i));
            item->addMsgid2(p2, p3 - p2);
            if (++i == _text.size())
            {
                fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
                goto done2;
            }
            p1 = _text[i];

            while (p1[0] == '\"')
            {
                p2 = p1 + 1;
                p3 = strrchr(p2, '\"');
                if (!p3)
                {
                    fprintf(stderr, "%s(%zu): Error: Malformed quoted string.\n", _path.c_str(), i + 1);
                    goto done2;
                }
                item->incMsgid2Line();
                item->addMsgid2(p2, p3 - p2);
                if (++i == _text.size())
                {
                    fprintf(stderr, "%s(%zu): Error: Unexpected EOF.\n", _path.c_str(), i + 1);
                    goto done2;
                }
                p1 = _text[i];
            }
        }

        if (!strncmp(p1, "msgstr \"", 8))
        {
            p2 = p1 + 8;
            p3 = strrchr(p2, '\"');
            if (!p3)
            {
                fprintf(stderr, "%s(%zu): Error: Malformed msgstr.\n", _path.c_str(), i + 1);
                goto done2;
            }
        }
        else if (!strncmp(p1, "msgstr[0] \"", 11))
        {
            p2 = p1 + 11;
            p3 = strrchr(p2, '\"');
            if (!p3)
            {
                fprintf(stderr, "%s(%zu): Error: Malformed msgstr[0].\n", _path.c_str(), i + 1);
                goto done2;
            }
        }
        else
        {
            fprintf(stderr, "%s(%zu): Error: Missing msgstr.\n", _path.c_str(), i + 1);
            goto done2;
        }
        item->setMsgstrLine(static_cast<int>(i));
        item->addMsgstr(p2, p3 - p2);
        if (++i < _text.size())
        {
            p1 = _text[i];
            while (p1[0] == '\"')
            {
                p2 = p1 + 1;
                p3 = strrchr(p2, '\"');
                if (!p3)
                {
                    fprintf(stderr, "%s(%zu): Error: Malformed quoted string.\n", _path.c_str(), i + 1);
                    goto done2;
                }
                item->incMsgstrLine();
                item->addMsgstr(p2, p3 - p2);
                if (++i == _text.size())
                {
                    break;
                }
                p1 = _text[i];
            }
        }

        if (!strncmp(p1, "msgstr[1] \"", 11))
        {
            p2 = p1 + 11;
            p3 = strrchr(p2, '\"');
            if (!p3)
            {
                fprintf(stderr, "%s(%zu): Error: Malformed msgstr[1].\n", _path.c_str(), i + 1);
                goto done2;
            }
            item->setMsgstr2Line(static_cast<int>(i));
            item->addMsgstr2(p2, p3 - p2);
            if (++i < _text.size())
            {
                p1 = _text[i];
                while (p1[0] == '\"')
                {
                    p2 = p1 + 1;
                    p3 = strrchr(p2, '\"');
                    if (!p3)
                    {
                        fprintf(stderr, "%s(%zu): Error: Malformed quoted string.\n", _path.c_str(), i + 1);
                        goto done2;
                    }
                    item->incMsgstr2Line();
                    item->addMsgstr2(p2, p3 - p2);
                    if (++i == _text.size())
                    {
                        break;
                    }
                    p1 = _text[i];
                }
            }
        }
        else if (item->msgid2Count())
        {
            fprintf(stderr, "%s(%zu): Error: Missing msgstr[1].\n", _path.c_str(), i + 1);
            goto done2;
        }

        _items.push_back(item);

        MsgMap::iterator iter = _msgidMap.find(item->msgid());
        if (iter == _msgidMap.end())
        {
            _msgidMap.insert(MsgEntry(item->msgid(), item));
        }
        else
        {
            fprintf(stderr, "%s(%u): Warning: Duplicate msgid: %s\n", _path.c_str(), item->msgidLine() + 1, item->msgid());
            delete item;
        }

        item = new MsgItem;
    }

done2:

    delete item;

    fprintf(log, "%s: %zu lines %zu entries\n",
            _path.c_str(), _text.size(), _items.size());

    return true;
}


void PoFile::close()
{
    _msgidMap.clear();

    for (size_t i = 0; i < _items.size(); i++)
    {
        delete _items[i];
    }
    _items.clear();

    for (size_t i = 0; i < _text.size(); i++)
    {
        free(_text[i]);
    }
    _text.clear();
}


void PoFile::merge(const PoFile& other)
{
    for (size_t i = 0; i < other._items.size(); i++)
    {
        MsgItem* item1 = other._items[i];
        for (int j = 0;; j++)
        {
            MsgItem* item2;
            if (j >= (int)_items.size())
            {
                item2 = new MsgItem(*item1);
                pushBack("\n");
                item2->setHeaderLine(static_cast<int>(_text.size()));
                pushBack(other._text[item1->headerLine() + 0]);
                for (j = 1; j < item1->headerCount(); j++)
                {
                    pushBack(other._text[item1->headerLine() + j]);
                    item2->incHeaderLine();
                }
                item2->setMsgidLine(static_cast<int>(_text.size()));
                pushBack(other._text[item1->msgidLine() + 0]);
                for (j = 1; j < item1->msgidCount(); j++)
                {
                    pushBack(other._text[item1->msgidLine() + j]);
                    item2->incMsgidLine();
                }
                if (item1->msgid2Count())
                {
                    item2->setMsgid2Line(static_cast<int>(_text.size()));
                    pushBack(other._text[item1->msgid2Line() + 0]);
                    for (j = 1; j < item1->msgid2Count(); j++)
                    {
                        pushBack(other._text[item1->msgid2Line() + j]);
                        item2->incMsgid2Line();
                    }
                }
                item2->setMsgstrLine(static_cast<int>(_text.size()));
                pushBack(other._text[item1->msgstrLine() + 0]);
                for (j = 1; j < item1->msgstrCount(); j++)
                {
                    pushBack(other._text[item1->msgstrLine() + j]);
                    item2->incMsgstrLine();
                }
                if (item1->msgstr2Count())
                {
                    item2->setMsgstrLine(static_cast<int>(_text.size()));
                    pushBack(other._text[item1->msgstr2Line() + 0]);
                    for (j = 1; j < item1->msgstr2Count(); j++)
                    {
                        pushBack(other._text[item1->msgstr2Line() + j]);
                        item2->incMsgstr2Line();
                    }
                }
                _items.push_back(item2);
                break;
            }
            item2 = _items[j];
            if (!strcmp(item2->msgid(), item1->msgid()))
            {
                int x = item2->headerLine();
                while (!strncmp(_text[x + 1], "#: ", 3))
                {
                    x++;
                }
                std::string s;
                s = _text[x];
                for (j = 0; j < item1->locationCount(); j++)
                {
                    s += item1->location(j);
                }
                replaceLine(x, s.c_str());
                break;
            }
        }
    }
}


bool PoFile::updateWithTemplate(const PoFile& latest)
{
    int updated = 0;

    std::string path1, path2;

    createFileNamesForUpdate(path1, path2);

    FILE* fp = fopen(path1.c_str(), "w");
    if (!fp)
    {
        char* msg = strerror(errno);
        fprintf(stderr, "%s: Error: Unable to open for writing: %s\n", path1.c_str(), msg);
        return false;
    }

    if (!_items.size())
    {
        print(fp);
        goto done;
    }

    _items[0]->updateRevisionDate(*this);

    print(fp, *_items[0]);

    for (size_t j = 1; j < latest._items.size(); j++)
    {
        fprintf(fp, "\n");
        const MsgItem item = *latest._items[j];
        MsgMap::const_iterator i = _msgidMap.find(item.msgid());
        if (i != _msgidMap.end())
        {
            latest.print(fp, item, *this, *i->second);
        }
        else
        {
            latest.print(fp, item);
            updated++;
        }
    }

    if (updated > 1)
    {
        fprintf(stderr, "%s: %d entries updated.\n", _path.c_str(), updated);
    }
    else if (updated == 1)
    {
        fprintf(stderr, "%s: 1 entry updated.\n", _path.c_str());
    }
    else
    {
        fprintf(stderr, "%s: 0 entry updated.\n", _path.c_str());
    }

done:

    fclose(fp);

    return updateFile(path1, path2);
}


bool PoFile::translateWithId()
{
    int translated = 0;

    std::string path1, path2;

    createFileNamesForUpdate(path1, path2);

    FILE* fp = fopen(path1.c_str(), "w");
    if (!fp)
    {
        char* msg = strerror(errno);
        fprintf(stderr, "%s: Error: Unable to open for writing: %s\n", path1.c_str(), msg);
        return false;
    }

    if (!_items.size())
    {
        print(fp);
        goto done;
    }

    _items[0]->updateRevisionDate(*this);

    print(fp, *_items[0]);

    for (size_t j = 1; j < _items.size(); j++)
    {
        fprintf(fp, "\n");
        MsgItem& item = *_items[j];
        if (*item.msgstr())
        {
            print(fp, item);
        }
        else
        {
            print(fp, item, true);
            translated++;
        }
    }

    if (translated > 1)
    {
        fprintf(stderr, "%d items translated.\n", translated);
    }
    else 
    {
        fprintf(stderr, "%d item translated.\n", translated);
    }

done:

    fclose(fp);

    return updateFile(path1, path2);
}


void PoFile::printUntranslated()
{
    int found = 0;
    for (size_t j = 1; j < _items.size(); j++)
    {
        MsgItem& item = *_items[j];
        if (!*item.msgstr())
        {
            if (found)
            {
                fprintf(stdout, "----------------------------------------------------------------------\n");
            }
            for (int i = 0; i < item.headerCount(); i++)
            {
                fprintf(stdout, "%7d %s", item.headerLine() + i + 1, _text[item.headerLine() + i]);
            }
            for (int i = 0; i < item.msgidCount(); i++)
            {
                fprintf(stdout, "%7d %s", item.msgidLine() + i + 1, _text[item.msgidLine() + i]);
            }
            for (int i = 0; i < item.msgid2Count(); i++)
            {
                fprintf(stdout, "%7d %s", item.msgid2Line() + i + 1, _text[item.msgid2Line() + i]);
            }
            found++;
        }
    }
    if (found > 1)
    {
        fprintf(stdout, "%d items found.\n", found);
    }
    else 
    {
        fprintf(stdout, "%d item found.\n", found);
    }
}


void PoFile::print(FILE* fp) const
{
    for (size_t i = 0; i < _text.size(); i++)
    {
        fprintf(fp, "%s", _text[i]);
    }
}


void PoFile::print(FILE* fp, const MsgItem& item, bool copyId) const
{
    for (int i = 0; i < item.headerCount(); i++)
    {
        fprintf(fp, "%s", _text[item.headerLine() + i]);
    }
    for (int i = 0; i < item.msgidCount(); i++)
    {
        fprintf(fp, "%s", _text[item.msgidLine() + i]);
    }
    for (int i = 0; i < item.msgid2Count(); i++)
    {
        fprintf(fp, "%s", _text[item.msgid2Line() + i]);
    }
    if (copyId)
    {
        if (item.msgid2Count())
        {
            char* q = strchr(_text[item.msgidLine()], '\"');
            fprintf(fp, "msgstr[0] %s", q);
            for (int i = 1; i < item.msgidCount(); i++)
            {
                fprintf(fp, "%s", _text[item.msgidLine() + i]);
            }
            q = strchr(_text[item.msgid2Line()], '\"');
            fprintf(fp, "msgstr[1] %s", q);
            for (int i = 1; i < item.msgid2Count(); i++)
            {
                fprintf(fp, "%s", _text[item.msgid2Line() + i]);
            }
        }
        else
        {
            char* q = strchr(_text[item.msgidLine()], '\"');
            fprintf(fp, "msgstr %s", q);
            for (int i = 1; i < item.msgidCount(); i++)
            {
                fprintf(fp, "%s", _text[item.msgidLine() + i]);
            }
        }
    }
    else
    {
        for (int i = 0; i < item.msgstrCount(); i++)
        {
            fprintf(fp, "%s", _text[item.msgstrLine() + i]);
        }
        for (int i = 0; i < item.msgstr2Count(); i++)
        {
            fprintf(fp, "%s", _text[item.msgstr2Line() + i]);
        }
    }
}


void PoFile::print(FILE* fp, const MsgItem& item, const PoFile& other, const MsgItem& itemForMsgstr) const
{
    for (int i = 0; i < item.headerCount(); i++)
    {
        fprintf(fp, "%s", _text[item.headerLine() + i]);
    }
    for (int i = 0; i < item.msgidCount(); i++)
    {
        fprintf(fp, "%s", _text[item.msgidLine() + i]);
    }
    for (int i = 0; i < item.msgid2Count(); i++)
    {
        fprintf(fp, "%s", _text[item.msgid2Line() + i]);
    }
    for (int i = 0; i < itemForMsgstr.msgstrCount(); i++)
    {
        fprintf(fp, "%s", other._text[itemForMsgstr.msgstrLine() + i]);
    }
    for (int i = 0; i < itemForMsgstr.msgstr2Count(); i++)
    {
        fprintf(fp, "%s", other._text[itemForMsgstr.msgstr2Line() + i]);
    }
}


void PoFile::pushBack(const char* s)
{
    char* t = strdup(s);
    if (t)
    {
        _text.push_back(t);
    }
    else
    {
        throw std::bad_alloc();
    }
}


void PoFile::replaceLine(size_t i, const char* s)
{
    char* t = strdup(s);
    if (t)
    {
        free(_text[i]);
        _text[i] = t;
    }
    else
    {
        throw std::bad_alloc();
    }
}


void PoFile::createFileNamesForUpdate(std::string& temp, std::string& backup)
{
    char buf[32];
    sprintf(buf, "%lu", (long)time(NULL));
    temp = _path;
    temp += ".";
    temp += buf;
    backup = _path;
    backup += ".backup";
}


bool PoFile::updateFile(const std::string& temp, const std::string& backup)
{
    if (unlink(backup.c_str()))
    {
        if (errno != ENOENT)
        {
            char* msg = strerror(errno);
            fprintf(stderr, "%s: Error: Unable to delete: %s\n", backup.c_str(), msg);
            goto rollback1;
        }
    }

    if (rename(_path.c_str(), backup.c_str()))
    {
        char* msg = strerror(errno);
        fprintf(stderr, "%s: Error: Unable to rename to \"%s\": %s\n", _path.c_str(), backup.c_str(), msg);
        goto rollback1;
    }

    if (rename(temp.c_str(), _path.c_str()))
    {
        char* msg = strerror(errno);
        fprintf(stderr, "%s: Error: Unable to rename to \"%s\": %s\n", temp.c_str(), _path.c_str(), msg);
        goto rollback2;
    }

    return open();

rollback2:

    if (rename(backup.c_str(), _path.c_str()))
    {
        char* msg = strerror(errno);
        fprintf(stderr, "%s: Error: Unable to rename to \"%s\": %s\n", backup.c_str(), _path.c_str(), msg);
    }

rollback1:

    if (unlink(temp.c_str()))
    {
        char* msg = strerror(errno);
        fprintf(stderr, "%s: Error: Unable to delete: %s\n", temp.c_str(), msg);
    }

    return false;
}
