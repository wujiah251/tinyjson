#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <initializer_list>
#include <type_traits>
#include <iostream>

using std::enable_if;
using std::initializer_list;
using std::is_convertible;
using std::is_floating_point;
using std::is_integral;
using std::is_same;
using std::map;
using std::string;
using std::vector;

namespace myJson
{
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
            vector<json> *Array;
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
        // 序列化构造函数
        json(initializer_list<json> list) : json()
        {
            SetType(Class::Object);
            for (auto i = list.begin(), e = list.end(); i != e; i++, i++)
                operator[](i->toString()) = *std::next(i);
        }
        // 左值复制构造函数
        json(const json &other)
        {
            switch (other.Type)
            {
            case Class::Object:
                Data.Map = new map<string, json>(other.Data.Map->begin(), other.Data.Map->end());
                break;
            case Class::Array:
                Data.Array = new vector<json>(other.Data.Array->begin(), other.Data.Array->end());
                break;
            case Class::String:
                Data.String = new string(*other.Data.String);
                break;
            default:
                Data = other.Data;
            }
            Type = other.Type;
        }
        // 右值复制构造函数
        json(json &&other) : Data(other.Data), Type(other.Type)
        {
            other.Type = Class::Null;
            other.Data.Map = nullptr;
        }
        // 析构函数
        ~json()
        {
            switch (Type)
            {
            case Class::Array:
                delete Data.Array;
                break;
            case Class::Object:
                delete Data.Map;
                break;
            case Class::String:
                delete Data.String;
                break;
            default:;
            }
        }
        // 重载赋值函数
        // 复制右值
        json &operator=(json &&other);
        // 复制左值
        json &operator=(const json &other);
        // 根据类型返回实例
        static json Make(Class type)
        {
            json ret;
            ret.SetType(type);
            return ret;
        }
        // 根据字符串解析出json对象
        static json Load(const string &);

        // 提供给array的函数
        template <typename T>
        void append(T arg);
        template <typename T, typename... U>
        void append(T arg, U... args);

        // 利用type_traits技法实现的构造函数
        // bool类型
        template <typename T>
        json(T b, typename enable_if<is_same<T, bool>::value>::type * = 0)
            : Data(b), Type(Class::Boolean)
        {
        }
        // 整型
        template <typename T>
        json(T i, typename enable_if<is_integral<T>::value && !is_same<T, bool>::value>::type * = 0)
            : Data((long)i), Type(Class::Integral)
        {
        }
        // 浮点数
        template <typename T>
        json(T f, typename enable_if<is_floating_point<T>::value>::type * = 0)
            : Data(double(f)), Type(Class::Floating)
        {
        }
        // 字符串类型
        template <typename T>
        json(T s, typename enable_if<is_convertible<T, string>::value>::type * = 0)
            : Data(string(s)), Type(Class::String)
        {
        }
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
            {
                Data.Array->resize(index + 1);
            }
            return Data.Array->operator[](index);
        }
        // 获取长度信息
        int length() const
        {
            if (Type == Class::Array)
            {
                return Data.Array->size();
            }
            else
                return -1;
        }
        // 获取规模信息
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
        // 获取类型信息
        Class jsonType() const { return Type; }
        // 判空
        bool isNull() const { return Type == Class::Null; }
        // 返回数据成员
        // 转成字符串类型
        string toString() const
        {
            return (Type == Class::String) ? std::move(json_escape(*Data.String)) : string("");
        }
        // 转成浮点数
        double toFloat() const
        {
            return (Type == Class::String) ? Data.Float : 0.0;
        }
        // 转成整型
        long toInt() const
        {
            return (Type == Class::Integral) ? Data.Int : 0;
        }
        // 转成布尔类型
        bool toBool() const
        {
            return (Type == Class::Boolean) ? Data.Bool : false;
        }

        // 生成可打印的字符串格式
        // depth是递归深度，tab是缩进大小，然后
        string dump(int depth = 1, string tab = "  ") const
        {
            string pad = "";
            for (int i = 0; i < depth; ++i, pad += tab)
                ;
            switch (Type)
            {
            case Class::Null:
                return "null";
            case Class::Object:
            {
                string s = "{\n";
                bool skip = true;
                for (auto &p : *Data.Map)
                {
                    if (!skip)
                        s += ",\n";
                    s += (pad + "\"" + p.first + "\" : " + p.second.dump(depth + 1, tab));
                    skip = false;
                }
                s += ("\n" + pad.erase(0, 2) + "}");
                return s;
            }
            case Class::Array:
            {
                string s = "[";
                bool skip = true;
                for (auto &p : *Data.Array)
                {
                    if (!skip)
                        s += ",";
                    s += p.dump(depth + 1, tab);
                    skip = false;
                }
                s += "]";
                return s;
            }
            case Class::String:
                return "\"" + json_escape(*Data.String) + "\"";
            case Class::Floating:
                return std::to_string(Data.Float);
            case Class::Integral:
                return std::to_string(Data.Int);
            case Class::Boolean:
                return Data.Bool ? "true" : "false";
            default:
                return "";
            }
            return "";
        }
        // 按照字符串格式输出json对象
        friend std::ostream &operator<<(std::ostream &os, const json &jsonObject)
        {
            os << jsonObject.dump();
            return os;
        }

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
                Data.Array = new vector<json>();
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

    // 向数组添加元素
    template <typename T>
    void json::append(T arg)
    {
        SetType(Class::Array);
        Data.Array->emplace_back(arg);
    }
    template <typename T, typename... U>
    void json::append(T arg, U... args)
    {
        append(arg);
        append(args...);
    }

    // 重载赋值函数
    // 右值赋值
    json &json::operator=(json &&other)
    {
        ClearData();
        Data = other.Data;
        Type = other.Type;
        other.Type = Class::Null;
        other.Data.Map = nullptr;
        return *this;
    }
    // 左值赋值
    json &json::operator=(const json &other)
    {
        ClearData();
        switch (other.Type)
        {
        case Class::Object:
            Data.Map = new map<string, json>(other.Data.Map->begin(), other.Data.Map->end());
            break;
        case Class::Array:
            Data.Array = new vector<json>(other.Data.Array->begin(), other.Data.Array->end());
            break;
        case Class::String:
            Data.String = new string(*other.Data.String);
            break;
        default:
            Data = other.Data;
        }
        Type = other.Type;
        return *this;
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
    // 获取一个字典
    json Object()
    {
        return std::move(json::Make(json::Class::Object));
    }
    // 这个名称空间里面是解析使用的函数
    namespace
    {
        // 先声明parseNext
        json parseNext(const string &str, size_t &offset);

        // 跳过前面的空白符
        void consumeWs(const string &str, size_t &offset)
        {
            // 判断是不是空白符：
            // ' '、'\t'、'\n'、'\v'、'\f'、'\r'
            while (isspace(str[offset]))
                ++offset;
        }

        // 通过有限状态机来进行解析。
        // 主状态机为parseNext，其余为从状态机。
        // 解析对象（key-value）
        json parseObject(const string &str, size_t &offset)
        {
            json res = json::Make(json::Class::Object);
            ++offset;
            consumeWs(str, offset);
            if (str[offset] == '}')
            {
                ++offset;
                return std::move(res);
            }
            while (true)
            {
                json key = parseNext(str, offset);
                consumeWs(str, offset);
                if (str[offset] != ':')
                {
                    std::cout << "Error: Pharse object failed!" << std::endl;
                    break;
                }
                consumeWs(str, ++offset);
                json value = parseNext(str, offset);
                res[key.toString()] = value;
                consumeWs(str, offset);
                if (str[offset] == ',')
                {
                    ++offset;
                    continue;
                }
                else if (str[offset] == '}')
                {
                    ++offset;
                    break;
                }
                else
                {
                    std::cout << "Error: Pharse object failed!" << std::endl;
                    break;
                }
            }
            return std::move(res);
        }
        // 解析数组
        json parseArray(const string &str, size_t &offset)
        {
            json res = json::Make(json::Class::Array);
            unsigned index = 0;
            ++offset;
            consumeWs(str, offset);
            if (str[offset] == ']')
            {
                ++offset;
                return std::move(res);
            }
            while (true)
            {
                res[index++] = parseNext(str, offset);
                consumeWs(str, offset);
                if (str[offset] == ',')
                {
                    ++offset;
                    continue;
                }
                else if (str[offset] == ']')
                {
                    ++offset;
                    break;
                }
                else
                {
                    std::cout << "Error: Parse array failed!" << std::endl;
                    return std::move(json::Make(json::Class::Array));
                }
            }
            return std::move(res);
        }
        // 解析字符串
        json parseString(const string &str, size_t &offset)
        {
            json res;
            string val;
            for (char c = str[++offset]; c != '\"'; c = str[++offset])
            {
                // 额外转义过的字符
                if (c == '\\')
                {
                    switch (str[++offset])
                    {
                    case '\"':
                        val += '\"';
                        break;
                    case '\\':
                        val += '\\';
                        break;
                    case '/':
                        val += '/';
                        break;
                    case 'b':
                        val += '\b';
                        break;
                    case 'f':
                        val += '\f';
                        break;
                    case 'n':
                        val += '\n';
                        break;
                    case 'r':
                        val += '\r';
                    case 't':
                        val += '\t';
                        break;
                    case 'u':
                    {
                        val += "\\u";
                        for (unsigned i = 1; i <= 4; ++i)
                        {
                            c = str[offset + i];
                            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                                val += c;
                            else
                            {
                                std::cout << "Error: Parse string failed!" << std::endl;
                                return std::move(json::Make(json::Class::String));
                            }
                        }
                        offset += 4;
                    }
                    break;
                    default:
                        val += '\\';
                        break;
                    }
                }
                else
                    val += c;
            }
            ++offset;
            res = val;
            return std::move(res);
        }
        // 解析数字（浮点数或者整型数）
        json parseNumber(const string &str, size_t &offset)
        {
            json res;
            string val;
            char c;
            bool isDouble = false;
            // 先把内容读入到val中
            while (true)
            {
                c = str[offset++];
                if ((c == '-') || (c >= '0' && c <= '9'))
                    val += c;
                else if (c == '.')
                {
                    val += c;
                    isDouble = true;
                }
                else
                {
                    break;
                }
            }
            if (!isspace(c) && c != '\0' && c != ',' && c != ']' && c != '}')
            {
                std::cout << "Error: Parse number failed" << std::endl;
                return std::move(json::Make(json::Class::Null));
            }
            --offset;
            if (isDouble)
            {
                res = std::stod(val);
            }
            else
            {
                res = std::stol(val);
            }
            return std::move(res);
        }
        // 解析布尔值
        json parseBool(const string &str, size_t &offset)
        {
            json res;
            if (str.substr(offset, 4) == "true")
                res = true;
            else if (str.substr(offset, 5) == "false")
                res = false;
            else
            {
                std::cerr << "Error: Pharse boolean failed!" << std::endl;
                return std::move(json::Make(json::Class::Null));
            }
            offset += (res.toBool() ? 4 : 5);
            return std::move(res);
        }
        // 解析null
        json parseNull(const string &str, size_t &offset)
        {
            json res;
            if (str.substr(offset, 4) != "null")
            {
                std::cout << "Error: Pharse null failed!" << std::endl;
                return std::move(res);
            }
            offset += 4;
            return std::move(res);
        }
        // 主状态机
        json parseNext(const string &str, size_t &offset)
        {
            char value;
            consumeWs(str, offset);
            value = str[offset];
            switch (value)
            {
            case '[':
                // array
                return std::move(parseArray(str, offset));
            case '{':
                // dictionary
                return std::move(parseObject(str, offset));
            case '\"':
                // "string"
                return std::move(parseString(str, offset));
            case 't':
            case 'f':
                // true or false
                return std::move(parseBool(str, offset));
            case 'n':
                //null
                return std::move(parseNull(str, offset));
            default:
                if ((value <= '9' && value >= '0') ||
                    value == '-')
                    return std::move(parseNumber(str, offset));
            }
            std::cout << "Error: Parse Next failed!" << std::endl;
            return json();
        }

    }
    // 解析json字符串
    json json::Load(const string &str)
    {
        size_t offset = 0;
        return std::move(parseNext(str, offset));
    }
}
#endif