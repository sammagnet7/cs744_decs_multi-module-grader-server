#include <iostream>

using namespace std;

void my_runtime_error(){
    long n;
    n = 100000000000;
 
    // 'n' is out of bound for
    // the array limit
    long a[n]; 
    cout << a[1] << " ";
}

int main()
{
    for(int i=1;i<=10;i++)  //for loop used to increament each time loop runs.
    {
        cout<<i<<" ";
    }
    //cout<<endl;
    //my_runtime_error();
    return 0;
}