#include "../json.h"
#include <iostream>
using namespace std;

using myJson::json;

int main()
{
    json object;
    object.Load("[1,2,3,4]");
    cout << object << endl;
    return 0;
}