#include <stdio.h>
#include <fstream>
#include <thread>
#include "chip8.h"

Chip8 chip8;

void run()
{
  chip8.run();
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s program-file [instructions_per_step]\n", argv[0]);
    abort();
  }
  std::ifstream program(argv[1]);
  
  if (argc >= 3)
  {
    chip8.instructions_per_step = atoi(argv[2]);
  }
  printf("Running at %d instructions per step\n", chip8.instructions_per_step);

  chip8.loadProgram(program);
  std::thread exec(run);
  chip8.initDisplay();

  return 0;
}
