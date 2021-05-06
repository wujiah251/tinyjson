#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <cmath>
#include <deque>
#include <map>
#include <initializer_list>

using std::deque;
using std::initializer_list;
using std::map;
using std::string;

string json_escape(const string &str)
{
    string output;
    for (unsigned i = 0; i < str.length(); ++i)
    {
        switch (str[i])
        {
        case '\"':
            output += "\\\"";
            break;
        case '\\':
            output += "\\\\";
            break;
        case '\b':
            output += "\\b";
            break;
        case '\f':
            output += "\\f";
            break;
        case '\n':
            output += "\\n";
            break;
        case '\r':
            output += "\\r";
            break;
        case '\t':
            output += "\\t";
            break;
        default:
            output += str[i];
            break;
        }
    }
    return std::move(output);
}

class json
{
private:
    union BackingData
    {
        BackingData(double d) : Float(d) {}
        BackingData(long l) : Int(l) {}
        BackingData(bool b) : Bool(b) {}
        BackingData(string s) : String(new string(s)) {}
        BackingData() : Int(0) {}
        deque<json> *Array;
        map<string, json> *Map;
        string *String;
        double Float;
        long Int;
        bool Bool;
    } Data;

public:
    enum class Class
    {
        Null,
        Object,
        Array,
        String,
        Floating,
        Integral,
        Boolean
    };
    // 构造函数
    json() : Data(), Type(Class::Null) {}
    json(initializer_list<json> list);
    // 复制构造函数
    json(const json &other);
    json(json &&other);
    // 析构函数
    ~json();
    // 重载复制函数
    json &operator=(json &other);
    json &operator=(const json &other);

    // 根据key获取value
    json &operator[](const string &key)
    {
        SetType(Class::Object);
        return Data.Map->operator[](key);
    }
    // 根据index获取value
    json &operator[](unsigned index)
    {
        SetType(Class::Array);
        if (index >= Data.Array->size())
            Data.Array->resize(index + 1);
        return Data.Array->operator[](index);
    }

    int length() const
    {
        if (Type == Class::Array)
        {
            return Data.Array->size();
        }
        else
            return -1;
    }
    int size() const
    {
        if (Type == Class::Object)
        {
            return Data.Map->size();
        }
        else if (Type == Class::Array)
        {
            return Data.Array->size();
        }
    }

    Class jsonType() const { return Type; }
    bool isNull() const { return Type == Class::Null; }
    // 返回数据成员
    string toString() const
    {
        return (Type == Class::String) ? std::move(json_escape(*Data.String)) : string("");
    }
    double toFloat() const
    {
        return (Type == Class::String) ? Data.Float : 0.0;
    }
    long ToInt() const
    {
        return (Type == Class::Integral) ? Data.Int : 0;
    }
    bool ToBool() const
    {
        return (Type == Class::Boolean) ? Data.Bool : false;
    }
    friend std::ostream &operator<<(std::ostream &, const json &);

private:
    // 根据类型重新创建对象（分配空间）
    void SetType(Class type)
    {
        if (type == Type)
        {
            return;
        }
        ClearData();
        switch (type)
        {
        case Class::Null:
            Data.Map = nullptr;
            break;
        case Class::Object:
            Data.Map = new map<string, json>();
            break;
        case Class::Array:
            Data.Array = new deque<json>();
            break;
        case Class::String:
            Data.String = new string();
            break;
        case Class::Floating:
            Data.Float = 0.0;
            break;
        case Class::Integral:
            Data.Int = 0;
            break;
        case Class::Boolean:
            Data.Bool = false;
            break;
        }
        Type = type;
    }
    /*
    根据类型释放空间，
    */
    void
    ClearData()
    {
        switch (Type)
        {
        case Class::Object:
            delete Data.Map;
            break;
        case Class::Array:
            delete Data.Array;
            break;
        case Class::String:
            delete Data.String;
            break;
        }
    }
    Class Type = Class::Null;
};

json::json(initializer_list<json> list) : json()
{
    SetType(Class::Object);
    for (auto i = list.begin(), e = list.end(); i != e; ++i, ++i)
        operator[](i->toString()) = *std::next(i);
}

#endif