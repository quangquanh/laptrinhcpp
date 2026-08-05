#include <iostream>

using namespace std;

int main()
{
    int temp;
    cout << "Hay nhap nhiet do" << endl;
    cin >> temp;

    if (temp < 15)
    {
        cout << "Lanh" << endl;
    }
    else if (temp < 24)
    {
        cout << "Mat me" << endl;
    }
    else if (temp < 32)
    {
        cout << "Am ap" << endl;
    }
    else
    {
        cout << "Nong" << endl;
    }
}