#include <iostream>
#include <fstream>
#include <bits/stdc++.h>

using namespace std;

int main(int argc, char* argv[])
{
    string in=argv[1];
    string test=in+"-pr-m.txt";
    string py=in+"-pr-p.txt";
    string java=in+"-pr-j.txt";

    fstream testfile(test);
    fstream pyfile(py);
    fstream javafile(java);

    double a,b,c,diff;
    string x;
    bool  b1=true,b2=true;
    while (testfile >> x)
    {
        testfile >> x;
        if(!(testfile >> x)){
            break;
        }
        a=stod(x);
        pyfile >> x;
        pyfile >> x;
        pyfile >> x;
        b=stod(x);
        javafile >> x;
        javafile >> x;
        javafile >> x;
        c=stod(x);
        diff=abs(a-b);
        if(diff>0.000001)
        {
            b1=false;
        }
        diff=abs(a-c);
        if(diff>0.000001)
        {
            b2=false;
        }
    }
    if(b1||b2)
    {
        cout << "Tested OK!";
    }
    else
    {
        cout << "Some Problem";
    }
    
}