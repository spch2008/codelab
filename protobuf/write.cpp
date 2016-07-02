/**
* @file   write.cpp
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-02 17:42:55
* @brief
**/

#include "person.pb.h"
#include <fstream>
using std::ofstream;

int main()
{
    Person person;
    person.set_name("spch2008");
    person.set_id(12);
    person.set_email("spch2008@foxmail.com");

    ofstream fout("pb_info.dat", std::ios::out | std::ios::binary); 
    person.SerializeToOstream(&fout);

    return 0;
}
