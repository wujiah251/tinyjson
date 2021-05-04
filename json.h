#include <string>
#include <cmath>
#include <deque>
#include <map>
#include <initializer_list>

using std::deque;
using std::map;
using std::string;

class json
{
    union BackingData
    {
        BackingData(double d) : Float(d) {}
        BackingData(long l) : Int(l) {}
        BackingData(bool b) : Bool(b) {}
        BackingData(string s) : String(new string(s)) {}
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
    // 复制构造函数
    json(const json &other);
    // 析构函数
    ~json();
    // 重载复制函数
    json &operator=(json &other);
    json &operator=(const json &other);

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
            Data.Object = new map<string, json>();
            break;
        case Class::Array:
            Data.Array = new deque<json>();
            break;
        case Class::String:
            Data.String = new stirng();
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