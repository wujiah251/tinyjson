#include "../json.h"
#include <iostream>
using namespace std;

using myJson::json;

int main()
{
    // null测试
    cout << json::Load("null") << endl;
    // bool测试
    cout << json::Load("true") << endl;
    // 整数测试
    cout << json::Load("12345") << endl;
    // 浮点数测试
    cout << json::Load("123.456") << endl;
    // 字符串测试
    cout << json::Load("\"string\"") << endl;
    // 空字典测试
    cout << json::Load("{}") << endl;
    // 字典测试
    cout << json::Load("{\"key\":\"value\"}") << endl;
    // 数组测试
    cout << json::Load("[1,2,3,4]") << endl;
    return 0;
}