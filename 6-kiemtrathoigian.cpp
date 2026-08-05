#include <iostream>

using namespace std;

int main()
{
    int hour;
    cout << "Hay nhap gio" << endl;
    cin >> hour;
    if (hour >= 0 && hour <= 23)
    {
        if (hour >= 5 && hour <= 11)
        {
            cout << "Buoi sang" << endl;
        }
        else if (hour >= 12 && hour <= 13)
        {
            cout << "Buoi trua" << endl;
        }
        else if (hour >= 14 && hour <= 17)
        {
            cout << "Buoi chieu" << endl;
        }
        else if (hour >= 17 && hour <= 18)
        {
            cout << "Buoi toi" << endl;
        }
        else
        {
            cout << "Nua dem";
        }
    }
    else
    {
        cout << "Gio khong hop le";
    }
}