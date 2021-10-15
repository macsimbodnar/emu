#include "console.hpp"
#include "mos6502.hpp"
#include "util.hpp"
#include <cstring>
#include <stdio.h>
#include <string>

#define RAM_SIZE 64 * 1024
#define EMPTY ' '

static Console *inst = nullptr;
static void cpu_log(const std::string &msg) {
  if (inst != nullptr) {
    inst->push_log(msg);
  }
}

static void mem_callback(void *ram, const uint16_t address,
                         const access_mode_t read_write, uint8_t &data) {

  uint8_t *RAM = (uint8_t *)ram;

  if (RAM == nullptr) {
    cpu_log("RAM is nullptr in the mem_callback");
    return;
  }

  switch (read_write) {
  case access_mode_t::READ:
    data = RAM[address];
    return;

  case access_mode_t::WRITE:
    RAM[address] = data;
    return;

  default:
    cpu_log("Unexpected memory access mod: " + std::to_string((int)read_write));
    return;
  }
}

Console::Console() : print_mem_page(0) {

  // Cleanup the screen
  for (unsigned int j = 0; j < HEIGHT; j++) {
    for (unsigned int i = 0; i < WIDTH; i++) {
      display[j][i] = EMPTY;

      if (j == 0 || i == WIDTH - 1 || j == CONTENT_HEIGHT - 1 ||
          j == HEIGHT - 1) {
        display[j][i] = '*';
      }
    }
  }

  memcpy(&(display[3][25]), "MEMORY", sizeof("MEMORY") - 1);
  memcpy(&(display[3][70]), "STATE", sizeof("STATE") - 1);
}

int Console::run(int argc, char **argv) {
  // Load the program from file to the memory
  FILE *file;
  long size;
  uint8_t RAM[RAM_SIZE];

  if (argc < 2) {
    printf("Provide the binary code!\n"); // Using printf because the dispaly is
                                          // not drawn yet
    return 1;
  }

  file = fopen(argv[1], "rb");

  if (file == nullptr) {
    printf("Can not open the file %s\n", argv[1]);
    return 1;
  }

  // get the file size
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);

  std::string line_2 =
      "Total memory:    " + std::to_string(mem_size) + " bytes";
  std::string line_3 = "Binary size:     " + std::to_string(size) + " bytes";

  set_header_line_2(line_2.c_str(), line_2.length());
  set_header_line_3(line_3.c_str(), line_3.length());

  // Load the code in RAM
  if (fread(RAM, sizeof(uint8_t), size, file) != (size_t)size) {
    printf("Failed read instructions from file.\n");
    return 1;
  }

  fclose(file);

  RAM[0xFFFC] = 0x20; // Set the reset Vector ll
  RAM[0xFFFD] = 0x40; // Set the reset Vector hh

  // Initialize the CPU
  MOS6502 cpu(mem_callback, (void *)RAM);
  cpu.set_log_callback(cpu_log);
  cpu.reset();

  // Set the class variables
  mem = RAM;
  mem_size = RAM_SIZE;
  current_state = cpu.get_status();

  // Enter into the main loop
  while (true) {
    draw_memory();
    draw_status();
    draw_exec_log();
    draw_logs();

    show();

    if (!get_input()) {
      break;
    }

    cpu.clock();
    current_state = cpu.get_status();
  }

  return 1;
}

void Console::draw_memory() {

  size_t from = print_mem_page * 256;
  size_t to = from + 256;

  size_t line = HEADER_HEIGHT;

  buff = "PAGE: ";

  if (print_mem_page < 10) {
    buff += "00";
  } else if (print_mem_page < 100) {
    buff += "0";
  }

  buff += std::to_string(print_mem_page);
  memcpy(&(display[line - 1][32]), &(buff[0]), buff.length());

  for (size_t i = from; i < to;) {

    size_t col = 0;
    col = sprintf(&(display[line][col]), "0x%04X  ",
                  ((unsigned int)i) & 0x0000FFFF);
    col--;

    for (size_t j = 0; j < MEM_WIDTH; j++, i++) {

      sprintf(&(display[line][col]), " %02X", mem[i]);

      if (i == current_state.address && i == current_state.PC) {
        sprintf(&(display[line][col]), "#%02X", mem[i]);
      } else if (i == current_state.address) {
        sprintf(&(display[line][col]), "$%02X", mem[i]);
      } else if (i == current_state.PC) {
        sprintf(&(display[line][col]), "*%02X", mem[i]);
      }

      col += 3;
    }

    display[line][col] = EMPTY;
    line++;
  }

  return;
}

void Console::draw_status() {
  unsigned int line = HEADER_HEIGHT;
  unsigned int col = STATUS_X;

  // FLAGS
  col += 5;
  buff = "NO-BDIZC   CYCLES: " + std::to_string(current_state.tot_cycles);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  line++;
  buff = uint8_to_bin(current_state.P);

  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // A register
  line += 2;
  col = STATUS_X;
  buff = "A: [" + uint8_to_bin(current_state.A) + "]         XXXX";
  sprintf(&(buff[22]), "0x%02X", current_state.A);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // X register
  line++;
  buff = "X: [" + uint8_to_bin(current_state.X) + "]         XXXX";
  sprintf(&(buff[22]), "0x%02X", current_state.X);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // Y register
  line++;
  buff = "Y: [" + uint8_to_bin(current_state.Y) + "]         XXXX";
  sprintf(&(buff[22]), "0x%02X", current_state.Y);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // S Stack pointer
  line++;
  buff = "S: [" + uint8_to_bin(current_state.S) + "]         XXXX";
  sprintf(&(buff[22]), "0x%02X", current_state.S);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // PC
  line++;
  col--;
  buff = "PC: [" + uint16_to_bin(current_state.PC) + "] XXXXXX";
  sprintf(&(buff[23]), "0x%04X", current_state.PC);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // Opcode
  line += 2;
  col = STATUS_X - 1;
  buff = "OP: [" + uint8_to_bin(current_state.opcode) + "]    " +
         current_state.opcode_name + "  XXXX";
  sprintf(&(buff[23]), "0x%02X", current_state.opcode);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // Fetched
  line++;
  col = STATUS_X - 2;
  buff = "BUS: [" + uint8_to_bin(current_state.data_bus) + "]         XXXX";
  sprintf(&(buff[24]), "0x%02X", current_state.data_bus);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // Current abbsolute address
  line += 2;
  col = STATUS_X - 2;
  buff = "ADR: [" + uint16_to_bin(current_state.address) + "] XXXXXX";
  sprintf(&(buff[24]), "0x%04X", current_state.address);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // Current relative address
  line++;
  buff = "REL: [" + uint16_to_bin(current_state.relative_adderess) + "] XXXXXX";
  sprintf(&(buff[24]), "0x%04X", current_state.relative_adderess);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // Tmp buffer
  line++;
  buff = "TMP: [" + uint16_to_bin(current_state.tmp_buff) + "] XXXXXX";
  sprintf(&(buff[24]), "0x%04X", current_state.tmp_buff);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());

  // Cycle counters
  line++;
  col = STATUS_X;
  buff = "CYCL N: ";

  if (current_state.cycles_count < 10) {
    buff += "0";
  }

  buff += std::to_string(current_state.cycles_count) + "   CYCL NEED: ";

  if (current_state.cycles_needed < 10) {
    buff += "0";
  }

  buff += std::to_string(current_state.cycles_needed);
  memcpy(&(display[line][col]), &(buff[0]), buff.length());
}

void Console::draw_exec_log() {
  // std::string s;
  // s.resize(50);
  // build_log_str(s, current_state.PC_executed);

  // memcpy(&(display[CONTENT_HEIGHT - 2][0]), &(s[0]), s.size());
  build_log_str(&(display[CONTENT_HEIGHT - 2][0]), current_state);
}

void Console::draw_logs() {
  size_t lin = CONTENT_HEIGHT;
  size_t limit = WIDTH - 1;

  for (size_t i = 0; i < LOG_LINES; i++) {
    // Clear the line
    memset(&(display[lin + i][0]), EMPTY, limit);

    // Write the log
    size_t to_draw = (log_head + i) % LOG_LINES;
    size_t size =
        (logs[to_draw].length() > limit) ? limit : logs[to_draw].length();
    memcpy(&(display[lin + i][0]), &(logs[to_draw][0]), size);
  }
}

void Console::set_header_line_2(const char *str, size_t size) {
  memcpy(&(display[1][0]), str, size);
}

void Console::set_header_line_3(const char *str, size_t size) {
  memcpy(&(display[2][0]), str, size);
}

void Console::show() {
  printf("\x1b[H\x1b[J"); // Erase the screen

  for (unsigned int j = 0; j < HEIGHT; j++) {
    printf("%.*s\n", WIDTH, &(display[j][0]));
  }
}

bool Console::get_input() {
  char in;
  int i = 0;

  while (true) {
    scanf(" %c", &in);

    switch (in) {
    case 'b': // Previous meme page
    case 'B':
      if (print_mem_page > 0) {
        print_mem_page--;
        draw_memory();
      }

      break;

    case 'c': // Next clock tick
    case 'C':
      return true;

    case 'l':
    case 'L':
      push_log("Some log " + std::to_string(i));
      i++;
      draw_logs();
      break;

    case 'n': // Next mem page
    case 'N':
      if (print_mem_page < 0x00FF) {
        print_mem_page++;
        draw_memory();
      }

      break;

    case 'q': // Quit
    case 'Q':
      return false;

    default:
      break;
    }

    show();
  }

  return false;
}

void Console::push_log(const std::string &str) {
  logs[log_head] = str;
  log_head++;

  if (log_head == LOG_LINES) {
    log_head = 0;
  }
}

int main(int argc, char **argv) {
  Console console;
  inst = &console;
  return console.run(argc, argv);
}
