#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <istream>
#include <random>
#include <tuple>

#include "memory.h"
#include "audio.h"

class Chip8
{
  struct
  {
    uint16_t PC = 0x200;
    uint16_t I;
    uint8_t V[16];
    uint8_t timerD, timerS;
    uint8_t SP = 0;
    uint8_t hp_48_flags[8];
  } reg;

  Memory<0x1000> memory; // 4KB
  Audio audio;

  static const unsigned int width = 64;
  static const unsigned int height = 32;
  static const unsigned int extWidth = width*2;
  static const unsigned int extHeight = height*2;
  uint8_t display[height][width];
  uint8_t extDisplay[extHeight][extWidth];
  bool extendedMode = false;

  char *rom_file_name;
  std::mt19937 rng;

  void execute(uint16_t instruction);
  void update_timers();
  void print_registers();
  void print_screen();

public:
  Chip8() {
    // TODO is this needed here?
    rng.seed(std::random_device()());
  }
  void loadProgram(char *rom);
  void reset();
  void run();
  void step();
  std::tuple<unsigned int, unsigned int, uint8_t*> get_display();

  unsigned int instructions_per_step = 10;
  unsigned int scaleFactor = 20;
  bool keys[16];
  bool muted = false;
};

#endif
