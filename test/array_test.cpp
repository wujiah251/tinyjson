#include "../json.h"
#include <iostream>

using myJson::json;
using namespace std;

int main()
{
    json array;
    array[2] = "Test2";
    cout << array << endl;
    array[1] = "Test1";
    cout << array << endl;
    array[0] = "Test0";
    cout << array << endl;
    array[3] = "Test4";
    cout << array << endl;
    // Arrays can be nested:
    json Array2;
    Array2[2][0][1] = true;
    cout << Array2 << endl;
}
