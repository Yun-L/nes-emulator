#include <cstdio>

int RAM[0x0800];
// mapped to 0x0000 - 0x07FF in hardware
// 0x0800 - 0x0FFF, 0x1000 - 0x17FF, 0x1800 - 0x1FFF
// are mirrors of RAM addresses

int PPU_REGISTERS[0x8];
// mapped to 0x2000 - 0x2007 in hardware
// 0x2008 - 0x3FFF are mirrors of PPU addresses, repeats every 8 bytes

int APU_IO_REGISTERS[0x18];
// mapped to 0x4000 - 0x4017 in hardware

int CARTRIDGE[0xBFE0];
// mapped to 0x4020 - 0xFFFF


int main(int argc, char *argv[]) {
    std::printf("nes emulator");
    return 0;
}
