/**
* @file   read.cpp
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-02 17:48:19
* @brief
**/

#include <iostream>
#include <fstream>
#include "person.pb.h"

using namespace std;

int main()
{
    ifstream fin("pb_info.dat", std::ios::in | std::ios::binary);

    Person person;
    person.ParseFromIstream(&fin);

    cout << "name: " << person.name() << endl;
    cout << "email: " << person.email() << endl;

    return 0;
}
