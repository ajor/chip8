#include <stdio.h>
#include <unistd.h>
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
  printf("  -m  Mute audio\n");
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
  while ((c = getopt(argc, argv, "i:s:m")) != -1)
  {
    switch (c)
    {
      case 'i':
        chip8.instructions_per_step = atoi(optarg);
        break;
      case 's':
        chip8.scaleFactor = atoi(optarg);
        break;
      case 'm':
        chip8.muted = true;
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
  chip8.loadProgram(rom);

  printf("Running at %d instructions per step\n", chip8.instructions_per_step);
  std::thread exec(run);
  chip8.initDisplay();

  chip8.running = false;
  exec.join();

  return 0;
}
