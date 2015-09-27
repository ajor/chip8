#include "memory.h"
#include <stdio.h>
#include <random>

const int memSize = 1000;
std::mt19937 rng; // not seeded - should be the same every time
std::uniform_int_distribution<std::mt19937::result_type> rand_byte(0, 0xff);

bool test_memory_8bit()
{
  bool pass = true;
  Memory<memSize> mem;

  // Initialize array
  uint8_t values[memSize];
  for (int i=0; i<memSize; i++)
  {
    values[i] = rand_byte(rng);
  }

  // Save to memory
  for (int i=0; i<memSize; i++)
  {
    mem.set8(i, values[i]);
  }

  // Read from memory and compare
  for (int i=0; i<memSize; i++)
  {
    uint8_t val = mem.get8(i);
    if (val != values[i]) {
      fprintf(stderr, "8-bit memory error: %u != %u\n", val, values[i]);
      pass = false;
    }
  }
  return pass;
}

bool test_memory_16bit()
{
  bool pass = true;
  Memory<memSize> mem;

  // Initialize array
  uint16_t values[memSize];
  for (int i=0; i<memSize; i+=2)
  {
    values[i] = rand_byte(rng);
  }

  // Save to memory
  for (int i=0; i<memSize; i+=2)
  {
    mem.set16(i, values[i]);
  }

  // Read from memory and compare
  for (int i=0; i<memSize; i+=2)
  {
    uint16_t val = mem.get16(i);
    if (val != values[i]) {
      fprintf(stderr, "16-bit memory error: %u != %u\n", val, values[i]);
      pass = false;
    }
  }
  return pass;
}

int main()
{
  bool result = true;
  result &= test_memory_8bit();
  result &= test_memory_16bit();
  if (result)
  {
    printf("All memory tests passed\n");
    return 0;
  }
  else
  {
    printf("Memory tests failed\n");
    return 1;
  }
}
