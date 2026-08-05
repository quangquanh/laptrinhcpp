#include <iostream>
using namespace std;

int main()
{
    int choice;

    cout << "===== MENU =====" << endl;
    cout << "1. Pho" << endl;
    cout << "2. Bun cha" << endl;
    cout << "3. Com tam" << endl;
    cout << "4. Thoat" << endl;

    cout << "Nhap lua chon: ";
    cin >> choice;

    switch (choice)
    {
    case 1:
        cout << "Ban da chon: Pho";
        break;

    case 2:
        cout << "Ban da chon: Bun cha";
        break;

    case 3:
        cout << "Ban da chon: Com tam";
        break;

    case 4:
        cout << "Thoat chuong trinh.";
        break;

    default:
        cout << "Lua chon khong hop le.";
    }

    return 0;
}