// Copyright (C) 2018 Hideaki Narita


#ifndef HNRT_JSON_H
#define HNRT_JSON_H


#include <stdio.h>
#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>
#include <vector>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class Json
        : public RefObj
    {
    public:

        enum Type
        {
            UNDEFINED = 65536,
            OBJECT,
            ARRAY,
            NUMBER,
            STRING,
            BOOLEAN,
            NULLVALUE,
        };

        enum Character
        {
            BEGIN_OBJECT = '{',
            END_OBJECT = '}',
            BEGIN_ARRAY = '[',
            END_ARRAY = ']',
            NAME_SEPARATOR = ':',
            VALUE_SEPARATOR = ',',
            SPACE = 0x20, // whitespace
            HORIZONTAL_TAB = 0x09, // whitespace
            LINE_FEED = 0x0a, // whitespace
            CARRIAGE_RETURN = 0x0d, // whitespace
        };

        class Member
            : public RefObj
        {
        public:

            static inline RefPtr<Member> create(const Glib::ustring&);
            static inline RefPtr<Member> create(const Glib::ustring&, const RefPtr<Json>&);

            inline ~Member();
            inline const Glib::ustring& key() const;
            inline const RefPtr<Json>& value() const;
            inline RefPtr<Json>& value();

        private:

            inline Member(const Glib::ustring&);
            inline Member(const Glib::ustring&, const RefPtr<Json>&);
            Member(const Member&);
            void operator =(const Member&);

            Glib::ustring _key;
            RefPtr<Json> _value;
        };

        typedef std::vector<RefPtr<Member> > MemberArray;

        typedef std::vector<RefPtr<Json> > Array;

        static RefPtr<Json> load(FILE*);
        static inline RefPtr<Json> create();
        static inline RefPtr<Json> create(Type);
        static inline RefPtr<Json> create(Type, const char*);
        static inline RefPtr<Json> create(const Glib::ustring&);
        static inline RefPtr<Json> create(const char*);
        static inline RefPtr<Json> create(long);
        static inline RefPtr<Json> create(int);
        static inline RefPtr<Json> create(double);
        static inline RefPtr<Json> create(bool);

        ~Json();
        void save(FILE*);
        void add(const RefPtr<Member>&);
        void add(const RefPtr<Json>&);
        inline Type type() const;
        const Glib::ustring& string() const;
        Glib::ustring& string();
        bool isInteger() const;
        bool isFloatingPoint() const;
        long integer() const;
        double floatingPoint() const;
        bool boolean() const;
        const MemberArray& members() const;
        const Array& array() const;
        Array& array();
        const RefPtr<Json>& find(const Glib::ustring&) const;
        RefPtr<Json>& find(const Glib::ustring&);
        bool get(const char*, Glib::ustring&) const;
        bool get(const char*, long&) const;
        bool get(const char*, int&) const;
        bool get(const char*, double&) const;
        bool get(const char*, bool&) const;
        bool get(const char*, const sigc::slot1<void, const RefPtr<Json>&>&) const;
        void set(const char*, const Glib::ustring&);
        void set(const char*, long);
        void set(const char*, int);
        void set(const char*, double);
        void set(const char*, bool);
        Array& setArray(const char*);

    private:

        Json();
        Json(Type);
        Json(Type, const char*);
        Json(const Glib::ustring&);
        Json(const char*);
        Json(long);
        Json(int);
        Json(double);
        Json(bool);
        Json(const Json&);
        void operator =(const Json&);

        Type _type;
        Glib::ustring _string;
        MemberArray _members;
        Array _array;
    };

    inline RefPtr<Json> Json::create()
    {
        return RefPtr<Json>(new Json);
    }

    inline RefPtr<Json> Json::create(Type type)
    {
        return RefPtr<Json>(new Json(type));
    }

    inline RefPtr<Json> Json::create(Type type, const char* value)
    {
        return RefPtr<Json>(new Json(type, value));
    }

    inline RefPtr<Json> Json::create(const Glib::ustring& value)
    {
        return RefPtr<Json>(new Json(value));
    }

    inline RefPtr<Json> Json::create(const char* value)
    {
        return RefPtr<Json>(new Json(value));
    }

    inline RefPtr<Json> Json::create(long value)
    {
        return RefPtr<Json>(new Json(value));
    }

    inline RefPtr<Json> Json::create(int value)
    {
        return RefPtr<Json>(new Json(value));
    }

    inline RefPtr<Json> Json::create(double value)
    {
        return RefPtr<Json>(new Json(value));
    }

    inline RefPtr<Json> Json::create(bool value)
    {
        return RefPtr<Json>(new Json(value));
    }
    
    inline Json::Type Json::type() const
    {
        return _type;
    }

    inline RefPtr<Json::Member> Json::Member::create(const Glib::ustring& key)
    {
        return RefPtr<Json::Member>(new Json::Member(key));
    }

    inline RefPtr<Json::Member> Json::Member::create(const Glib::ustring& key, const RefPtr<Json>& value)
    {
        return RefPtr<Json::Member>(new Json::Member(key, value));
    }

    inline Json::Member::Member(const Glib::ustring& key)
        : _key(key)
    {
    }

    inline Json::Member::Member(const Glib::ustring& key, const RefPtr<Json>& value)
        : _key(key)
        , _value(value)
    {
    }

    inline Json::Member::~Member()
    {
    }

    inline const Glib::ustring& Json::Member::key() const
    {
        return _key;
    }

    inline const RefPtr<Json>& Json::Member::value() const
    {
        return _value;
    }

    inline RefPtr<Json>& Json::Member::value()
    {
        return _value;
    }
}


#endif //!HNRT_JSON_H
