#include "../json.h"
#include <iostream>
#include <cstddef>

using myJson::json;
using namespace std;

int main()
{
    json obj({"Key", 1, "Key3", true, "Key4", nullptr, "Key2", {"Key4", "VALUE", "Arr", myJson::Array(1, "Str", false)}});
    cout << obj << endl;
}
