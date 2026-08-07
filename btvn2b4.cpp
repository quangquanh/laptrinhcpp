#include <iostream>
#include <string>
using namespace std;

int main()
{
    string hoTen;
    string lop;

    cout << "Nhap ho ten: ";
    getline(cin, hoTen);

    cout << "Nhap lop hoc: ";
    getline(cin, lop);

    cout << "Xin chao, " << hoTen << ", lop " << lop << "!" << endl;

    return 0;
}
