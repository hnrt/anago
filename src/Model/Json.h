// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_JSON_H
#define HNRT_JSON_H


#include <stdio.h>
#include <glibmm.h>
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
            VALUE_FALSE,
            VALUE_NULL,
            VALUE_TRUE,
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

            Object()
            {
            }

            const MemberArray& members() const
            {
                return _members;
            }

            MemberArray& members()
            {
                return _members;
            }

            void add(const char* key, bool value)
            {
                RefPtr<Member> member(new Member());
                member->set(Glib::ustring(key));
                RefPtr<Value> valptr(new Value());
                valptr->set(value ? VALUE_TRUE : VALUE_FALSE);
                member->set(valptr);
                _members.push_back(member);
            }

            void add(const char* key, long value)
            {
                RefPtr<Member> member(new Member());
                member->set(Glib::ustring(key));
                RefPtr<Value> valptr(new Value());
                valptr->set(value);
                member->set(valptr);
                _members.push_back(member);
            }

            void add(const char* key, const char* value)
            {
                RefPtr<Member> member(new Member());
                member->set(Glib::ustring(key));
                RefPtr<Value> valptr(new Value());
                valptr->set(Glib::ustring(value));
                member->set(valptr);
                _members.push_back(member);
            }

            void add(const char* key, const RefPtr<Object>& value)
            {
                RefPtr<Member> member(new Member());
                member->set(Glib::ustring(key));
                RefPtr<Value> valptr(new Value());
                valptr->set(value);
                member->set(valptr);
                _members.push_back(member);
            }

            void add(const char* key, const Array& value)
            {
                RefPtr<Member> member(new Member());
                member->set(Glib::ustring(key));
                RefPtr<Value> valptr(new Value());
                for (Array::const_iterator iter = value.begin(); iter != value.end(); iter++)
                {
                    valptr->array().push_back(*iter);
                }
                member->set(valptr);
                _members.push_back(member);
            }

        private:

            Object(const Object&);
            void operator =(const Object&);
            
            MemberArray _members;
        };

        class Value
            : public RefObj
        {
        public:

            Value()
                : _type(UNDEFINED)
            {
            }

            Type type() const
            {
                return _type;
            }

            void set(Type type)
            {
                _type = type;
            }

            const RefPtr<Object>& object() const
            {
                return _object;
            }

            void set(const RefPtr<Object>& object)
            {
                _type = OBJECT;
                _object = object;
            }

            const Array& array() const
            {
                return _array;
            }

            Array& array()
            {
                _type = ARRAY;
                return _array;
            }

            long number() const
            {
                return _number;
            }

            void set(long number)
            {
                _type = NUMBER;
                _number = number;
            }

            const Glib::ustring& string() const
            {
                return _string;
            }

            void set(const Glib::ustring& string)
            {
                _type = STRING;
                _string = string;
            }

        private:

            Type _type;
            RefPtr<Object> _object;
            Array _array;
            long _number;
            Glib::ustring _string;
        };

        class Member
            : public RefObj
        {
        public:

            Member()
            {
            }

            Member(const Glib::ustring& key, const RefPtr<Value>& value)
                : _key(key)
                , _value(value)
            {
            }

            const Glib::ustring& key() const
            {
                return _key;
            }

            void set(const Glib::ustring& key)
            {
                _key = key;
            }

            const RefPtr<Value>& value() const
            {
                return _value;
            }

            void set(const RefPtr<Value>& value)
            {
                _value = value;
            }

        private:

            Member(const Member&);
            void operator =(const Member&);

            Glib::ustring _key;
            RefPtr<Value> _value;
        };

        Json();
        virtual ~Json();
        virtual void load(FILE*);
        virtual void save(FILE*);
        virtual void set(Type type) { _type = type; }
        virtual void set(const RefPtr<Object>&);
        virtual const Array& array() const { return _array; }
        virtual Array& array();

    protected:

        Json(const Json&);
        void operator =(const Json&);
        void write(FILE*, const RefPtr<Object>&, int);
        void write(FILE*, const Array&, int);
        void write(FILE*, const RefPtr<Member>&, int);
        void write(FILE*, const RefPtr<Value>&, int);
        void indent(FILE*, int);

        Type _type;
        RefPtr<Object> _object;
        Array _array;
    };
}


#endif //!HNRT_JSON_H
