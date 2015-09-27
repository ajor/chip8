#include "memory.h"
#include <stdio.h>

bool test_memory()
{
  bool pass = true;
  int memSize = 1000;
  Memory mem(memSize);
  for (int i=0; i<=memSize; i++)
  {
    mem.set(i, i);
  }
  for (int i=0; i<=memSize; i++)
  {
    uint8_t val = mem.get(i);
    uint8_t expected = (uint8_t)i;
    if (val != expected) {
      fprintf(stderr, "Memory error: %u != %u\n", val, expected);
      pass = false;
    }
  }
  return pass;
}

int main()
{
  bool result = test_memory();
  if (result)
  {
    printf("Memory tests passed\n");
    return 0;
  }
  else
  {
    printf("Memory tests failed\n");
    return 1;
  }
}
