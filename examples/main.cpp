#include <iostream>
#include <memory>
#include <cassert>
#include <fcntl.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
  FILE *fp = fopen("./0.idx", "a+");
  if (fp == nullptr)
  {
    std::cout << "fp is null" << std::endl;
    return 1;
  }
  std::string s = "abc";
  fwrite(s.c_str(), 1, s.length(), fp);
  return 0;
}