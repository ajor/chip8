#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <thread>
#include "chip8.h"

static Chip8 chip8;
static char *name;

void run()
{
  chip8.run();
}

void usage()
{
  printf("Usage: %s [options] rom\n", name);
  printf("Options:\n");
  printf("  -i  Instructions per step (default: 10)\n");
  printf("  -s  Screen scale factor (default: 20)\n");
}

int main(int argc, char* argv[])
{
  name = argv[0];
  if (argc < 2)
  {
    usage();
    abort();
  }
  int c;
  while ((c = getopt(argc, argv, "i:s:")) != -1)
  {
    switch (c)
    {
      case 'i':
        chip8.instructions_per_step = atoi(optarg);
        break;
      case 's':
        chip8.scaleFactor = atoi(optarg);
        break;
      default:
        usage();
        abort();
    }
  }
  if (optind != argc-1)
  {
    usage();
    abort();
  }

  char *rom = argv[optind];
  std::ifstream program(rom);

  if (!program.is_open())
  {
    fprintf(stderr, "Couldn't load ROM from '%s'\n", rom);
    abort();
  }

  printf("Running at %d instructions per step\n", chip8.instructions_per_step);

  chip8.loadProgram(program);
  std::thread exec(run);
  chip8.initDisplay();

  chip8.running = false;
  exec.join();

  return 0;
}
