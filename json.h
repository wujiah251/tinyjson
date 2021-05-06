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
    // 包装器
    template <typename Container>
    class jsonWrapper
    {
        Container *object;

    public:
        jsonWrapper(Container *val) : object(val) {}
        jsonWrapper(std::nullptr_t) : object(nullptr) {}
        typename Container::iterator begin()
        {
            return object ? object->begin() : typename Container::iterator();
        }
        typename Container::iterator end()
        {
            return object ? object->end() : typename Container::iterator();
        }
        typename Container::const_iterator begin() const
        {
            return object ? object->begin() : typename Container::iterator();
        }
        typename Container::const_iterator end() const
        {
            return object ? object->end() : typename Container::iterator();
        }
    };
    template <typename Container>
    class jsonConstWrapper
    {
        const Container *object;

    public:
        jsonConstWrapper(const Container *val) : object(val) {}
        jsonConstWrapper(std::nullptr_t) : object(nullptr) {}
        typename Container::const_iterator begin() const
        {
            return object ? object->begin() : typename Container::const_iterator();
        }
        typename Container::const_iterator end() const
        {
            return object ? object->end() : typename Container::const_iterator();
        }
    };

    // 构造函数
    json() : Data(), Type(Class::Null)
    {
    }
    json(initializer_list<json> list);
    // 复制构造函数
    json(const json &other);
    json(json &&other);
    // 析构函数
    ~json();

    // 根据类型返回实例
    static json Make(Class type)
    {
        json ret;
        ret.SetType(type);
        return ret;
    }

    // 重载复制函数
    json &operator=(json &&other);
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
    void ClearData()
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

// 列表初始化
json::json(initializer_list<json> list) : json()
{
    SetType(Class::Object);
    for (auto i = list.begin(), e = list.end(); i != e; ++i, ++i)
        operator[](i->toString()) = *std::next(i);
}
// 左值复制构造函数
json::json(const json &other)
{
    switch (other.Type)
    {
    case Class::Object:
        Data.Map = new map<string, json>(other.Data.Map->begin(), other.Data.Map->end());
        break;
    case Class::Array:
        Data.Array = new deque<json>(other.Data.Array->begin(), other.Data.Array->end());
        break;
    case Class::String:
        Data.String = new string(*other.Data.String);
    default:
        Data = other.Data;
    }
    Type = other.Type;
}
// 右值复制构造函数
json::json(json &&other)
    : Data(other.Data), Type(other.Type)
{
    other.Type = Class::Null;
    other.Data.Map = nullptr;
}
// 析构函数
json::~json()
{
    ClearData();
}

// 重载赋值函数
json::json &json::operator=(json &&other)
{
    ClearData();
    Data = other.Data;
    Type = other.Type;
    other.ClearData();
    return *this;
}
json::json &json::operator=(const json &other)
{
    ClearData();
    switch (other.Type)
    {
    case Class::Object:
        Data.Map = new map<string, json>(other.Data.Map->begin(), other.Data.Map->end());
        break;
    case Class::Array:
        Data.Array = new deque<json>(other.Data.Array->begin(), other.Data.Array->end());
        break;
    case Class::String:
        Data.String = new string(*other.Data.String);
    default:
        Data = other.Data;
    }
    Type = other.Type;
}

// 获得不同类型的实例
// 获取一个数组
json Array()
{
    return std::move(json::Make(json::Class::Array));
}
template <typename... T>
json Array(T... args)
{
    json arr = json::Make(json::Class::Array);
    arr.append(args...);
    return std::move(arr);
}
json Object()
{
    return std::move(json::Make(json::Class::Object));
}
std::ostream &operator<<(std::ostream &os, const json &json)
{
    os << json.dump();
    return os;
}
#endif