#include "chip8.h"
#include "font_loader.h"

#include <istream>
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <string.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

std::uniform_int_distribution<std::mt19937::result_type> rand_byte(0, 0xff);
GLFWwindow *window;
GLuint shader_program;

bool keys[16];
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void Chip8::loadProgram(std::istream& program)
{
  memory.load(0x200, 0x1000-0x200, program);
//  memory.print(0x200-1, 20);

  // 5-bit (4x5 pixel) font
  uint8_t font[] = {0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
                    0x20, 0x60, 0x20, 0x20, 0x70, // 1
                    0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
                    0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
                    0x90, 0x90, 0xf0, 0x10, 0x10, // 4
                    0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
                    0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
                    0xf0, 0x10, 0x20, 0x40, 0x40, // 7
                    0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
                    0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
                    0xf0, 0x90, 0xf0, 0x90, 0x90, // A
                    0xe0, 0x90, 0xe0, 0x90, 0xe0, // B
                    0xf0, 0x80, 0x80, 0x80, 0xf0, // C
                    0xe0, 0x90, 0x90, 0x90, 0xe0, // D
                    0xf0, 0x80, 0xf0, 0x80, 0xf0, // E
                    0xf0, 0x80, 0xf0, 0x80, 0x80};// F
  memory.load(0x100, 16*5, font);

  // 10-bit (8*10) font
  std::vector<uint8_t> font10 = load_font();
  memory.load(0x150, 10*10, font10.data());
}

void Chip8::run()
{
  while (1) {
    step();
  }
}

void Chip8::step()
{
  std::chrono::duration<int, std::ratio<1, 60>> tick(1);
  std::this_thread::sleep_for(tick);
  update_timers();
  for (unsigned int i=0; i<instructions_per_step; i++)
  {
    //printf("fetch: 0x%08X\n", reg.PC);
    uint16_t instruction = memory.get16(reg.PC);
    reg.PC += 2;
    //printf("execute: %04X\n", instruction);
    execute(instruction);
    //print_registers();
    //print_screen();
    //getchar();
  }
}

void Chip8::execute(uint16_t instruction)
{
  // Not the most efficient implementation - switch statements aren't
  // converted to jump tables... but it's easily fast enough for
  // Chip-8
  uint16_t bit1 = instruction & 0xf000;
  switch (bit1)
  {
    case 0x0000:
      {
        if ((instruction & 0x00f0) == 0x00c0)
        {
          // SUPER-CHIP
          // 00CN - SCD nibble
          // Scroll display N lines down
          printf("1\n");
          unsigned int n = (instruction & 0x000f);
          unsigned int nr, w, h;
          uint8_t *disp;
          if (extendedMode)
          {
            w = extWidth;
            h = extHeight;
            disp = &extDisplay[0][0];
          }
          else
          {
            w = width;
            h = height;
            disp = &display[0][0];
          }
          nr = n*w;
          memmove(disp + nr, disp, w*h - nr);
          memset(disp, 0, nr);
        }
        else
        {
          switch (instruction & 0x00ff)
          {
            case 0x00e0:
              {
                // 00E0 - CLS
                // Clear the display
                memset(display, 0, sizeof(display));
                break;
              }
            case 0x00ee:
              {
                // 00EE - RET
                // Return from a subroutine
                reg.PC = memory.get16(reg.SP);
                reg.SP -= 2;
                break;
              }
            case 0x00fb:
              {
                // SUPER-CHIP
                // 00FB - SCR
                // Scroll display 4 pixels right
                unsigned int w, h;
                uint8_t *disp;
                if (extendedMode)
                {
                  w = extWidth;
                  h = extHeight;
                  disp = &extDisplay[0][0];
                }
                else
                {
                  w = width;
                  h = height;
                  disp = &display[0][0];
                }
                for (unsigned int i=0; i<h; i++)
                {
                  memmove(disp+4, disp, w-4);
                  memset(disp, 0, 4);
                  disp += w; // Next row
                }
                break;
              }
            case 0x00fc:
              {
                // SUPER-CHIP
                // 00FC - SCL
                // Scroll display 4 pixels left
                unsigned int w, h;
                uint8_t *disp;
                if (extendedMode)
                {
                  w = extWidth;
                  h = extHeight;
                  disp = &extDisplay[0][0];
                }
                else
                {
                  w = width;
                  h = height;
                  disp = &display[0][0];
                }
                for (unsigned int i=0; i<h; i++)
                {
                  memmove(disp, disp+4, w-4);
                  memset(disp+w-4, 0, 4);
                  disp += w; // Next row
                }
                break;
              }
            case 0x00fd:
              {
                // SUPER-CHIP
                // 00FE - EXIT
                // Exit interpreter
                printf("Exiting...\n");
                abort(); // TODO nicer exit
                break;
              }
            case 0x00fe:
              {
                // SUPER-CHIP
                // 00FE - LOW
                // Disable extended screen mode
                extendedMode = false;
                break;
              }
            case 0x00ff:
              {
                // SUPER-CHIP
                // 00FF - HIGH
                // Enable extended screen mode for full-screen graphics
                extendedMode = true;
                break;
              }
            default:
              printf("Unknown instruction: %04X\n", instruction);
              abort();
          }
        }
        break;
      }
    case 0x1000:
      {
        // 1nnn - JP addr
        // Jump to location nnn
        reg.PC = instruction & 0x0fff;
        break;
      }
    case 0x2000:
      {
        // 2nnn - CALL addr
        // Call subroutine at nnn
        reg.SP += 2;
        memory.set16(reg.SP, reg.PC);
        reg.PC = instruction & 0x0fff;
        break;
      }
    case 0x3000:
      {
        // 3xkk - SE Vx, byte
        // Skip next instruction if Vx = kk
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int kk = (instruction & 0x00ff);
        if (reg.V[x] == kk)
          reg.PC += 2;
        break;
      }
    case 0x4000:
      {
        // 4xkk - SNE Vx, byte
        // Skip next instruction if Vx != kk
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int kk = (instruction & 0x00ff);
        if (reg.V[x] != kk)
          reg.PC += 2;
        break;
      }
    case 0x5000:
      {
        // 5xy0 - SE Vx, Vy
        // Skip next instruction if Vx = Vy
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int y = (instruction & 0x00f0)>>4;
        if (reg.V[x] == reg.V[y])
          reg.PC += 2;
        break;
      }
    case 0x6000:
      {
        // 6xkk - LD Vx, byte
        // Set Vx = kk
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int kk = (instruction & 0x00ff);
        reg.V[x] = kk;
        break;
      }
    case 0x7000:
      {
        // 7xkk - ADD Vx, byte
        // Set Vx = Vx + kk
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int kk = (instruction & 0x00ff);
        reg.V[x] += kk;
        break;
      }
    case 0x8000:
      {
        // 8xyo - (o) Vx, Vy
        // Set Vx = Vx (o) Vy
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int y = (instruction & 0x00f0)>>4;
        switch (instruction & 0x000f)
        {
          case 0: // LD Vx, Vy
            reg.V[x] = reg.V[y];
            break;
          case 1: // OR
            reg.V[x] |= reg.V[y];
            break;
          case 2: // AND
            reg.V[x] &= reg.V[y];
            break;
          case 3: // XOR
            reg.V[x] ^= reg.V[y];
            break;
          case 4: // ADD
            {
              if (reg.V[x] > 0xff - reg.V[y])
                reg.V[0xf] = 1;
              else
                reg.V[0xf] = 0;
              reg.V[x] += reg.V[y];
              break;
            }
          case 5: // SUB
            {
              if (reg.V[x] < reg.V[y])
                reg.V[0xf] = 0;
              else
                reg.V[0xf] = 1;
              reg.V[x] -= reg.V[y];
              break;
            }
          case 6: // SHR
            reg.V[0xf] = reg.V[x] & 0x0001;
            reg.V[x] >>= 1;
            break;
          case 7: // SUBN
            {
              if (reg.V[y] < reg.V[x])
                reg.V[0xf] = 0;
              else
                reg.V[0xf] = 1;
              reg.V[x] = reg.V[y] - reg.V[x];
              break;
            }
          case 0xe: // SHL
            reg.V[0xf] = reg.V[x] & 0x8000;
            reg.V[x] <<= 1;
            break;
          default:
            printf("Unknown instruction: %04X\n", instruction);
            abort();
        }
        break;
      }
    case 0x9000:
      {
        // 9xy0 - SNE Vx, Vy
        // Skip next instruction if Vx != Vy
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int y = (instruction & 0x00f0)>>4;
        if (reg.V[x] != reg.V[y])
          reg.PC += 2;
        break;
      }
    case 0xa000:
      {
        // Annn - LD I, addr
        // Set I = nnn
        reg.I = instruction & 0x0fff;
        break;
      }
    case 0xb000:
      {
        // Bnnn - JP V0, addr
        // Jump to location nnn + V0
        reg.PC = reg.V[0] + (instruction & 0x0fff);
        break;
      }
    case 0xc000:
      {
        // Cxkk - RND Vx, byte
        // Set Vx = random byte AND kk
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int kk = (instruction & 0x00ff);
        unsigned int r = rand_byte(rng);
        reg.V[x] = r & kk;
        break;
      }
    case 0xd000:
      {
        // Dxyn - DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
        // SUPER-CHIP If N=0 and extended mode, show 16x16 sprite.
        unsigned int x = (instruction & 0x0f00)>>8;
        unsigned int y = (instruction & 0x00f0)>>4;
        unsigned int n = (instruction & 0x000f);
        reg.V[0xf] = 0;
        if (extendedMode)
        {
          if (n == 0)
          {
            // Draw a 16x16 sprite
            for (unsigned int row=0; row<16; row++)
            {
              uint16_t sprite_row = memory.get16(reg.I + row*2);
              for (unsigned int col=0; col<16; col++)
              {
                if (sprite_row & (0x8000 >> col))
                {
                  if (extDisplay[(reg.V[y]+row)%extHeight][(reg.V[x]+col)%extWidth])
                  {
                    reg.V[0xf] = 1;
                    extDisplay[(reg.V[y]+row)%extHeight][(reg.V[x]+col)%extWidth] = 0;
                  }
                  else
                  {
                    extDisplay[(reg.V[y]+row)%extHeight][(reg.V[x]+col)%extWidth] = 0xff;
                  }
                }
              }
            }
          }
          else
          {
            // Draw an n-byte sprite in extended mode
            for (unsigned int row=0; row<n; row++)
            {
              uint8_t sprite_row = memory.get8(reg.I + row);
              for (unsigned int col=0; col<8; col++)
              {
                if (sprite_row & (0x80 >> col))
                {
                  if (extDisplay[(reg.V[y]+row)%extHeight][(reg.V[x]+col)%extWidth])
                  {
                    reg.V[0xf] = 1;
                    extDisplay[(reg.V[y]+row)%extHeight][(reg.V[x]+col)%extWidth] = 0;
                  }
                  else
                  {
                    extDisplay[(reg.V[y]+row)%extHeight][(reg.V[x]+col)%extWidth] = 0xff;
                  }
                }
              }
          }
          }
        }
        else
        {
          // Draw an n-byte sprite in normal mode
          for (unsigned int row=0; row<n; row++)
          {
            uint8_t sprite_row = memory.get8(reg.I + row);
            for (unsigned int col=0; col<8; col++)
            {
              if (sprite_row & (0x80 >> col))
              {
                if (display[(reg.V[y]+row)%height][(reg.V[x]+col)%width])
                {
                  reg.V[0xf] = 1;
                  display[(reg.V[y]+row)%height][(reg.V[x]+col)%width] = 0;
                }
                else
                {
                  display[(reg.V[y]+row)%height][(reg.V[x]+col)%width] = 0xff;
                }
              }
            }
          }
        }
        break;
      }
    case 0xe000:
      {
        unsigned int x = (instruction & 0x0f00)>>8;
        switch (instruction & 0x00ff)
        {
          case 0x009e:
            {
              // Ex9E - SKP Vx
              // Skip next instruction if key with the value of Vx is pressed
              if (keys[reg.V[x]])
                reg.PC += 2;
              break;
            }
          case 0x00a1:
            // ExA1 - SKNP Vx
            // Skip next instruction if key with the value of Vx is not pressed
            if (!keys[reg.V[x]])
              reg.PC += 2;
            break;
          default:
            printf("Unknown instruction: %04X\n", instruction);
            abort();
        }
        break;
      }
    case 0xf000:
      {
        unsigned int x = (instruction & 0x0f00)>>8;
        switch (instruction & 0x00ff)
        {
          case 0x0007:
            {
              // Fx07 - LD Vx, DT
              // Set Vx = delay timer value
              reg.V[x] = reg.timerD;
              break;
            }
          case 0x000a:
            {
              // Fx0A - LD Vx, K
              // Wait for a key press, store the value of the key in Vx
              bool key_pressed = false;
              for (unsigned int i=0; i<16; i++)
              {
                if (keys[i])
                {
                  reg.V[x] = i;
                  key_pressed = true;
                  break;
                }
              }
              if (!key_pressed)
                reg.PC -= 2; // stay on this instruction

              break;
            }
          case 0x0015:
            {
              // Fx15 - LD DT, Vx
              // Set delay timer = Vx
              reg.timerD = reg.V[x];
              break;
            }
          case 0x0018:
            {
              // Fx18 - LD ST, Vx
              // Set sound timer = Vx
              reg.timerS = reg.V[x];
              break;
            }
          case 0x001e:
            {
              // Fx1E - ADD I, Vx
              // Set I = I + Vx
              if (reg.I + reg.V[x] > 0xfff)
                reg.V[0xf] = 1;
              else
                reg.V[0xf] = 0;
              reg.I += reg.V[x];
              break;
            }
          case 0x0029:
            {
              // Fx29 - LD F, Vx
              // Point I to 5-byte font sprite for hex character VX
              reg.I = 0x100 + reg.V[x]*5; // 0x100 is address of '0' digit
              break;
            }
          case 0x0030:
            {
              // SUPER-CHIP
              // Fx30 - LD HF, Vx
              // Point I to 10-byte font sprite for digit VX (0..9)
              reg.I = 0x150 + reg.V[x]*10; // 0x150 is address of '0' digit
              break;
            }
          case 0x0033:
            {
              // Fx33 - LD B, Vx
              // Store BCD representation of Vx in memory locations I, I+1, and I+2
              memory.set8(reg.I,    reg.V[x]/100);
              memory.set8(reg.I+1, (reg.V[x]%100)/10);
              memory.set8(reg.I+2,  reg.V[x]%10);
              break;
            }
          case 0x0055:
            {
              // Fx55 - LD [I], Vx
              // Store registers V0 through Vx in memory starting at location I
              uint16_t I = reg.I;
              for (unsigned int j=0; j<=x; j++)
              {
                memory.set8(I, reg.V[j]);
                I += 1;
              }
              break;
            }
          case 0x0065:
            {
              // Fx65 - LD Vx, [I]
              // Read registers V0 through Vx from memory starting at location I
              uint16_t I = reg.I;
              for (unsigned int j=0; j<=x; j++)
              {
                reg.V[j] = memory.get8(I);
                I += 1;
              }
              break;
            }
//          case 0x0075:
//            {
//              // SUPER-CHIP
//              // Fx75 - LD R, Vx
//              // Store V0..VX in RPL user flags (X <= 7)
//              // TODO what does this mean??
//              break;
//            }
//          case 0x0085:
//            {
//              // SUPER-CHIP
//              // Fx85 - LD Vx, R
//              // Read V0..VX from RPL user flags (X <= 7)
//              // TODO what does this mean??
//              break;
//            }
          default:
            printf("Unknown instruction: %04X\n", instruction);
            abort();
        }
        break;
      }
    default:
      printf("Unknown instruction: %04X\n", instruction);
      abort();
  }
}

void Chip8::update_timers()
{
  if (reg.timerD > 0)
    --reg.timerD;
  if (reg.timerS > 0)
    --reg.timerS;
}

void Chip8::print_registers()
{
  for (int i=0; i<16; i++)
  {
    printf("V%X: %02X\n", i, reg.V[i]);
  }
  printf("tD: %02X\n", reg.timerD);
  printf("tS: %02X\n", reg.timerS);
  printf("I:  %04X\n", reg.I);
}

void Chip8::print_screen()
{
  unsigned int w, h;
  uint8_t *disp;
  if (extendedMode)
  {
    w = extWidth;
    h = extHeight;
    disp = &extDisplay[0][0];
  }
  else
  {
    w = width;
    h = height;
    disp = &display[0][0];
  }
  if (extendedMode)
  {
    printf("/--------------------------------------------------------------------------------------------------------------------------------\\\n");
  }
  else
  {
    printf("/----------------------------------------------------------------\\\n");
  }
  for (unsigned int y=0; y<h; y++)
  {
    printf("|");
    for (unsigned int x=0; x<w; x++)
    {
      if (*(disp+y*w+x))
        printf("0");
      else
        printf(" ");
    }
    printf("|\n");
  }
  if (extendedMode)
  {
    printf("\\--------------------------------------------------------------------------------------------------------------------------------/\n");
  }
  else
  {
    printf("\\----------------------------------------------------------------/\n");
  }
}

void Chip8::initDisplay()
{
  //
  // Set up window
  //
  unsigned int screenWidth  = width * scaleFactor;
  unsigned int screenHeight = height * scaleFactor;
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialise GLFW\n");
    abort();
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window = glfwCreateWindow(screenWidth, screenHeight, "Chip8 Emulator", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to open window.\n");
    glfwTerminate();
    abort();
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialise GLEW.\n");
    abort();
  }

  //
  // OpenGL settings
  //
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, screenWidth, screenHeight, 0);

  //
  // Shaders
  //
  const GLchar *vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec2 position;"
    "layout (location = 1) in vec2 texCoord;"
    "out vec2 TexCoord;"
    "void main()"
    "{"
    "  gl_Position = vec4(position, 0.0, 1.0);"
    "  TexCoord = texCoord;"
    "}";

  const GLchar *fragment_shader_source =
    "#version 330 core\n"
    "in vec2 TexCoord;"
    "out vec4 colour;"
    "uniform sampler2D display;"
    "void main()"
    "{"
    "  colour = texture(display, TexCoord);"
    "}";

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);
  GLint status;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
  if (!status)
  {
    char error_buffer[512];
    glGetShaderInfoLog(vertex_shader, 512, NULL, error_buffer);
    fprintf(stderr, "Vertex shader error:\n%s\n", error_buffer);
    abort();
  }

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
  if (!status)
  {
    char error_buffer[512];
    glGetShaderInfoLog(fragment_shader, 512, NULL, error_buffer);
    fprintf(stderr, "Fragment shader error:\n%s\n", error_buffer);
    abort();
  }

  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
  if (!status)
  {
    char error_buffer[512];
    glGetProgramInfoLog(shader_program, 512, NULL, error_buffer);
    fprintf(stderr, "Shader link error:\n%s\n", error_buffer);
    abort();
  }
  glUseProgram(shader_program);

  //
  // Buffers
  //
  GLfloat display_vertices[] = {
    // Pos          Tex
   -1.0f,-1.0f,  0.0f, 1.0f,
   -1.0f, 1.0f,  0.0f, 0.0f,
    1.0f,-1.0f,  1.0f, 1.0f,
    1.0f, 1.0f,  1.0f, 0.0f,
    1.0f,-1.0f,  1.0f, 1.0f,
   -1.0f, 1.0f,  0.0f, 0.0f,
  };

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(display_vertices), display_vertices, GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);

  // TexCoord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLvoid*)(2*sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  //
  // Texture
  //
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // Uncomment to change display FPS to confirm games still run at correct speed
  //  glfwSwapInterval(2);
  while (!glfwWindowShouldClose(window))
  {
    unsigned int w, h;
    uint8_t *disp;
    if (extendedMode)
    {
      w = extWidth;
      h = extHeight;
      disp = &extDisplay[0][0];
    }
    else
    {
      w = width;
      h = height;
      disp = &display[0][0];
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, disp);
    glUniform1i(glGetUniformLocation(shader_program, "display"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  bool pressed = (action != GLFW_RELEASE);
  switch (key)
  {
    case GLFW_KEY_1: keys[0x1] = pressed; break;
    case GLFW_KEY_2: keys[0x2] = pressed; break;
    case GLFW_KEY_3: keys[0x3] = pressed; break;
    case GLFW_KEY_4: keys[0xc] = pressed; break;
    case GLFW_KEY_Q: keys[0x4] = pressed; break;
    case GLFW_KEY_W: keys[0x5] = pressed; break;
    case GLFW_KEY_E: keys[0x6] = pressed; break;
    case GLFW_KEY_R: keys[0xd] = pressed; break;
    case GLFW_KEY_A: keys[0x7] = pressed; break;
    case GLFW_KEY_S: keys[0x8] = pressed; break;
    case GLFW_KEY_D: keys[0x9] = pressed; break;
    case GLFW_KEY_F: keys[0xe] = pressed; break;
    case GLFW_KEY_Z: keys[0xa] = pressed; break;
    case GLFW_KEY_X: keys[0x0] = pressed; break;
    case GLFW_KEY_C: keys[0xb] = pressed; break;
    case GLFW_KEY_V: keys[0xf] = pressed; break;
  }
}
