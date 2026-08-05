#include <iostream>
using namespace std;

int main()
{
    string password;

    cout << "Nhap mat khau: ";
    cin >> password;

    if (password == "123456")
        cout << "Dang nhap thanh cong!" << endl;
}