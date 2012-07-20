#include "chunk.hpp"
#include <stdexcept>
#include <iostream>
int main(int argc, char* argv[])
{
  chunk<int> chnk;
  int* values[64] = {0};

  if (chnk.bits!=0)
    throw std::exception();

  for ( int i = 0 ; i < 64 ; i++)
    values[i] = chnk.mark();

  if (chnk.bits!=0xFFFFFFFFFFFFFFFF)
    throw std::exception();

  for ( int i = 0 ; i < 64 ; i++)
    if ( values[i] != (int*)((char*)chnk.data + i * sizeof(int)) )
      throw std::exception();

  for ( int i = 0 ; i < 64 ; i+=2)
    chnk.free( values[i] );

  std::cout << std::hex << chnk.bits << std::endl;
  if (chnk.bits!=0xAAAAAAAAAAAAAAAA)
    throw std::exception();



  return 0;
}
