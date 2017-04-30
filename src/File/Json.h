// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_JSON_H
#define HNRT_JSON_H


#include <stdio.h>
#include <glibmm.h>
#include <sigc++/sigc++.h>
#include <vector>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class Json
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

        class Object;

        class Value;

        typedef std::vector<RefPtr<Value> > Array;

        class Member;

        typedef std::vector<RefPtr<Member> > MemberArray;

        class Object
            : public RefObj
        {
        public:

            Object();
            Object(const MemberArray&);
            ~Object();
            const MemberArray& members() const { return _members; }
            void add(const char*);
            void add(const char*, const char*);
            void add(const char*, long);
            void add(const char*, double);
            void add(const char*, bool);
            void add(const char*, const RefPtr<Object>&);
            void add(const char*, const Array&);
            const RefPtr<Value>& get(const char*) const;
            RefPtr<Value>& get(const char*);

        private:

            Object(const Object&);
            void operator =(const Object&);
            
            MemberArray _members;
        };

        class Value
            : public RefObj
        {
        public:

            Value();
            Value(Type);
            Value(Type, const char*);
            Value(const Glib::ustring&);
            Value(const char*);
            Value(long);
            Value(int);
            Value(double);
            Value(bool);
            Value(const RefPtr<Object>&);
            Value(const Array&);
            ~Value();
            Type type() const { return _type; }
            const Glib::ustring& string() const;
            bool isInteger() const;
            bool isFloatingPoint() const;
            long integer() const;
            double floatingPoint() const;
            bool boolean() const;
            const RefPtr<Object>& object() const;
            RefPtr<Object>& object();
            const Array& array() const;
            Array& array();

        private:

            Type _type;
            Glib::ustring _string;
            RefPtr<Object> _object;
            Array _array;
        };

        class Member
            : public RefObj
        {
        public:

            Member(const Glib::ustring&);
            Member(const Glib::ustring&, const RefPtr<Value>&);
            ~Member();
            const Glib::ustring& key() const { return _key; }
            const RefPtr<Value>& value() const { return _value; }
            RefPtr<Value>& value() { return _value; }

        private:

            Member(const Member&);
            void operator =(const Member&);

            Glib::ustring _key;
            RefPtr<Value> _value;
        };

        Json();
        ~Json();
        const RefPtr<Value>& root() const { return _root; }
        void set(const RefPtr<Value>& value) { _root = value; }
        void load(FILE*);
        void save(FILE*);
        bool getString(const char*, Glib::ustring&) const;
        bool getString(const RefPtr<Value>&, const char*, Glib::ustring&) const;
        bool getLong(const char*, long&) const;
        bool getLong(const RefPtr<Value>&, const char*, long&) const;
        bool getInt(const char*, int&) const;
        bool getInt(const RefPtr<Value>&, const char*, int&) const;
        bool getBoolean(const char*, bool&) const;
        bool getBoolean(const RefPtr<Value>&, const char*, bool&) const;
        bool getArray(const char*, const sigc::slot2<void, const Json&, const RefPtr<Value>&>&) const;
        bool getArray(const RefPtr<Value>&, const char*, const sigc::slot2<void, const Json&, const RefPtr<Value>&>&) const;
        void setString(const char*, const Glib::ustring&);
        void setString(RefPtr<Value>&, const char*, const Glib::ustring&);
        void setLong(const char*, long);
        void setLong(RefPtr<Value>&, const char*, long);
        void setInt(const char*, int);
        void setInt(RefPtr<Value>&, const char*, int);
        void setBoolean(const char*, bool);
        void setBoolean(RefPtr<Value>&, const char*, bool);
        Array& addArray(const char*);
        Array& addArray(RefPtr<Value>&, const char*);

    protected:

        Json(const Json&);
        void operator =(const Json&);

        RefPtr<Value> _root;
    };
}


#endif //!HNRT_JSON_H
