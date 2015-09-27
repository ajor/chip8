#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <istream>
#include <random>
#include <chrono>

#include "memory.h"

class Chip8
{
  struct
  {
    uint16_t PC = 0x200;
    uint16_t I;
    uint8_t V[16];
    uint8_t timerD, timerS;
    uint8_t SP = 0;
  } reg;

  uint8_t display[32][64];

  Memory<0x1000> memory; // 4KB

  void step();
  void execute(uint16_t instruction);
  void update_timers();
  void print_registers();
  void print_screen();

  std::mt19937 rng;

public:
  Chip8() {
    // TODO is this needed here?
    rng.seed(std::random_device()());
  }
  void loadProgram(std::istream& program);
  void run();
  void initDisplay();

  unsigned int instructions_per_step = 10;
};

#endif
