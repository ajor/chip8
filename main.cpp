#include <stdio.h>
#include <unistd.h>
#include "chip8.h"

static char *name;
static Chip8 chip8;

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
    return 1;
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
        return 1;
    }
  }
  if (optind != argc-1)
  {
    usage();
    return 1;
  }

  char *rom = argv[optind];
  chip8.loadProgram(rom);

  printf("Running at %d instructions per step\n", chip8.instructions_per_step);
  chip8.run();

  return 0;
}

#ifdef __EMSCRIPTEN__
extern "C"
{
  void set_instructions_per_step(int n)
  {
    chip8.instructions_per_step = n;
  }
}
#endif
