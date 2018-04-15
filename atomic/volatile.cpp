/**
* @file   volatile.cpp
* @author sunpengcheng(spch2008n@foxmail.com)
* @brief
**/
#include <iostream>
using namespace std;


void DumyCall() {
  cout << "dumy call" << endl;
}

void LoopCall() {
  cout << "dumy loop" << endl;
}

int main() {
  volatile int value = 15;
  //int value = 15;
  for (int i = 0; i < value; ++i) {
    LoopCall();
  }
}
