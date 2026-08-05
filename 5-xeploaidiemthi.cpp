#include <iostream>
using namespace std;

int main()
{
    float n;
    cout << "Nhap diem cua ban: ";
    cin >> n;
    if (n > 10 || n < 0)
    {
        cout << "Diem khong hop le!" << endl;
    }
    else
    {
        if (n >= 8)
        {
            cout << "Gioi" << endl;
        }
        else
        {
            if (n >= 6.5)
            {
                cout << "Kha" << endl;
            }
            else
            {
                if (n >= 5)
                {
                    cout << "Trung binh" << endl;
                }
                else
                {
                    cout << "Yeu" << endl;
                }
            }
        }
        return 0;
    }