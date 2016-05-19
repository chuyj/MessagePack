#ifndef MSGPACK_HPP
#define MSGPACK_HPP

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <typeinfo>
#include <vector>
#include "endian.h"

using namespace std;

struct value {
    public:
        typedef bool boolean_type;
        typedef std::int64_t integer_type;
        typedef std::string string_type;
        typedef std::vector<value> array_type;
        typedef std::map<string_type, value> map_type;

        value(){tag = tag_t::nil_tag;}
        explicit value(const boolean_type input){tag = tag_t::boolean_tag; new(&val.b) boolean_type(input);}
        explicit value(const integer_type input){tag = tag_t::integer_tag; new(&val.i) integer_type(input);}
        explicit value(const string_type& input){tag = tag_t::string_tag; new(&val.s) string_type(input);}
        explicit value(const array_type& input){tag = tag_t::array_tag; new(&val.a) array_type(input);}
        explicit value(const map_type& input){tag = tag_t::map_tag; new(&val.m) map_type(input);}

        value(const value& rhs){
            tag = rhs.tag;
            if (rhs.is_nil() == true){;}
            else if (rhs.is_boolean() == true){new(&val.b) boolean_type(rhs.val.b);}
            else if (rhs.is_integer() == true){new(&val.i) integer_type(rhs.val.i);}
            else if (rhs.is_string() == true){new(&val.s) string_type(rhs.val.s);}
            else if (rhs.is_array() == true){new(&val.a) array_type(rhs.val.a);}
            else if (rhs.is_map() ==true){new(&val.m) map_type(rhs.val.m);}
        }
        value& operator=(const value& rhs){
            //~value();
            if (tag == tag_t::nil_tag){;}
            else if (tag == tag_t::boolean_tag){val.b.~boolean_type();}
            else if (tag == tag_t::integer_tag){val.i.~integer_type();}
            else if (tag == tag_t::string_tag){val.s.~string_type();}
            else if (tag == tag_t::array_tag){val.a.~array_type();}
            else if (tag == tag_t::map_tag){val.m.~map_type();}
            tag = rhs.tag;
            if (rhs.is_nil() == true){}
            else if (rhs.is_boolean() == true){new(&val.b) boolean_type(rhs.val.b);}
            else if (rhs.is_integer() == true){new(&val.i) integer_type(rhs.val.i);}
            else if (rhs.is_string() == true){new(&val.s) string_type(rhs.val.s);}
            else if (rhs.is_array() == true){new(&val.a) array_type(rhs.val.a);}
            else if (rhs.is_map() ==true){new(&val.m) map_type(rhs.val.m);}
            return *this;
        }
        ~value(){
            if (is_nil() == true){}
            else if (is_boolean() == true){val.b.~boolean_type();}
            else if (is_integer() == true){val.i.~integer_type();}
            else if (is_string() == true){val.s.~string_type();}
            else if (is_array() == true){val.a.~array_type();}
            else if (is_map() ==true){val.m.~map_type();}
        }

        bool is_nil() const{return tag == tag_t::nil_tag;}
        bool is_boolean() const{return tag == tag_t::boolean_tag;}
        bool is_integer() const{return tag == tag_t::integer_tag;}
        bool is_string() const{return tag == tag_t::string_tag;}
        bool is_array() const{return tag == tag_t::array_tag;}
        bool is_map() const{return tag == tag_t::map_tag;}

        boolean_type& get_boolean(){if(!is_boolean())throw bad_cast(); return val.b;}
        integer_type& get_integer(){if(!is_integer())throw bad_cast(); return val.i;}
        string_type& get_string(){if(!is_string())throw bad_cast(); return val.s;}
        array_type& get_array(){if(!is_array())throw bad_cast(); return val.a;}
        map_type& get_map(){if(!is_map())throw bad_cast(); return val.m;}

        const boolean_type& get_boolean() const {if(!is_boolean())throw bad_cast(); return val.b;}
        const integer_type& get_integer() const {if(!is_integer())throw bad_cast(); return val.i;}
        const string_type& get_string() const {if(!is_string())throw bad_cast(); return val.s;}
        const array_type& get_array() const {if(!is_array())throw bad_cast(); return val.a;}
        const map_type& get_map() const {if(!is_map())throw bad_cast(); return val.m;}

        friend bool operator==(const value& lhs, const value& rhs){
            if (lhs.tag == rhs.tag){
                if (lhs.is_nil() == true){return true;}
                else if (lhs.is_boolean() == true){return lhs.val.b == rhs.val.b;} 
                else if (lhs.is_integer() == true){return lhs.val.i == rhs.val.i;}
                else if (lhs.is_string() == true){return lhs.val.s == rhs.val.s;}
                else if (lhs.is_array() == true){return lhs.val.a == rhs.val.a;}
                else if (lhs.is_map() == true){return lhs.val.m == rhs.val.m;}
            }
            return false;
        }
        friend bool operator!=(const value& lhs, const value& rhs){return !(lhs == rhs);}

        std::ostream& serialize(std::ostream& out) const{
            switch(tag)
            {
                case tag_t::nil_tag:
                    out << '\xc0';
                    return out;
                    break;
                case tag_t::boolean_tag:
                    out << (get_boolean() ? '\xc3' : '\xc2'); 
                    return out;
                    break;
                case tag_t::integer_tag:{
                    out << '\xd3';
                    int64_t x; x = int64_t(val.i);
                    x = htobe64(move(x));
                    out.write(reinterpret_cast<char*>(&x),sizeof(x));
                    return out;
                    break;
                }
                case tag_t::string_tag:{
                    out << '\xdb';
                    uint32_t x = uint32_t(val.s.length());
                    x = htobe32(move(x));
                    out.write(reinterpret_cast<char*>(&x),4);
                    out.write(val.s.c_str(), val.s.length());
                    return out;
                    break;
                }
                case tag_t::array_tag:{
                    out << '\xdd';
                    uint32_t x = uint32_t(val.a.size());
                    x = htobe32(move(x));
                    out.write(reinterpret_cast<char*>(&x),4);
                    for (size_t i = 0; i < val.a.size(); i++){
                        val.a[i].serialize(out);
                    }
                    return out;
                    break;
                }
                case tag_t::map_tag:{
                    out << '\xdf';
                    uint32_t x = uint32_t(val.m.size());
                    x = htobe32(move(x));
                    out.write(reinterpret_cast<char*>(&x),4);
                    for (auto&& i: val.m){
                        value(i.first).serialize(out);
                        value(i.second).serialize(out);
                    }
                    return out;
                    break;
                }
                default:
                    return out;
                    break;
            }

        }
        static value deserialize(std::istream& in){
            char* intag = new char();
            in.read(intag, 1);
            switch(*intag)
            {
                case '\xc0':
                    return value(); break;
                case '\xc2':
                    return value(false); break;
                case  '\xc3':
                    return value(true);break;
                case '\xd3':{
                    int64_t x;
                    in.read(reinterpret_cast<char*>(&x), sizeof(x));
                    if (in.fail()) return value();
                    x = be64toh(move(x));
                    return value(x);
                    break;
                }
                case '\xdb':{
                    std::string str;
                    uint32_t x;
                    in.read(reinterpret_cast<char*>(&x), sizeof(x));
                    if (in.fail()) return value();
                    x = be32toh(move(x));
                    char ch;
                    for (uint32_t i = 0; i < x; i++){
                        ch = in.get(); if (in.fail()) return value();
                        str += ch;
                    }
                    return value(str);
                    break;
                }
                case '\xdd':{
                    array_type tmp;
                    uint32_t x;
                    in.read(reinterpret_cast<char*>(&x),sizeof(x));
                    if (in.fail()) return value();
                    x = be32toh(move(x));
                    for (uint32_t i = 0; i < x; i++){
                        tmp.push_back(deserialize(in));
                        if (in.fail()) return value();
                    }
                    return value(tmp);
                    break;
                }
                case '\xdf':
                    map_type tmp;
                    uint32_t x;
                    in.read(reinterpret_cast<char*>(&x), sizeof(x));
                    if (in.fail()) return value();
                    x = be32toh(move(x));
                    for (uint32_t i = 0; i < x; i++){
                        char ch = in.peek();
                        if (ch != '\xdb') {in.setstate(std::ios::failbit); return value();}
                        value ttmp = deserialize(in);
                        if (in.fail()) return value();
                        tmp[ttmp.get_string()] = deserialize(in);
                        if (in.fail()) return value();
                    }
                    return value(tmp);
                    break;


            }
            in.setstate(std::ios::failbit);
            return value();
        }

    private:
        enum class tag_t { nil_tag, boolean_tag, integer_tag, string_tag,
                           array_tag, map_tag };
        union union_t {
            union_t() {}
            ~union_t() {}
            boolean_type b;
            integer_type i;
            string_type s;
            array_type a;
            map_type m;
        };

        tag_t tag;
        union_t val;
};

#endif
