#include <iostream>
using namespace std;

int main()
{
    int a, b;
    cout << "Nhap so thu nhat: ";
    cin >> a;
    cout << "Nhap so thu hai: ";
    cin >> b;

    if (a > b)
        cout << a << " lon hon " << b << endl;
    else
        cout << a << " khong lon hon " << b << endl;
}