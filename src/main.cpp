#include <cstdio>
#include <cstdint>

#define IS_NEG(uint8_num) (1 == uint8_num >> 7)
#define SET_C(b) ((STATUS_REG &= 11111110) + (bool)b)
#define SET_Z(b) ((STATUS_REG &= 11111101) + ((bool)b << 1))
#define SET_I(b) ((STATUS_REG &= 11111011) + ((bool)b << 2))
#define SET_D(b) ((STATUS_REG &= 11110111) + ((bool)b << 3))
#define SET_V(b) ((STATUS_REG &= 10111111) + ((bool)b << 6))
#define SET_N(b) ((STATUS_REG &= 01111111) + ((bool)b << 7))
#define GET_C() ((STATUS_REG & 00000001) > 0)
#define GET_Z() ((STATUS_REG & 00000010) > 0)
#define GET_I() ((STATUS_REG & 00000100) > 0)
#define GET_D() ((STATUS_REG & 00001000) > 0)
#define GET_V() ((STATUS_REG & 01000000) > 0)
#define GET_N() ((STATUS_REG & 10000000) > 0)

uint16_t PC;
uint8_t STACK_POINTER;
uint8_t ACCUMULATOR;
uint8_t IND_REG_X;
uint8_t IND_REG_Y;
uint8_t STATUS_REG;

uint8_t MEMORY[0xFFFF];
// mapped to 0x0000 - 0x07FF in hardware
// 0x0800 - 0x0FFF, 0x1000 - 0x17FF, 0x1800 - 0x1FFF
// are mirrors of RAM addresses
// mapped to 0x2000 - 0x2007 in hardware
// 0x2008 - 0x3FFF are mirrors of PPU addresses, repeats every 8 bytes
// mapped to 0x4000 - 0x4017 in hardware
// mapped to 0x4020 - 0xFFFF

void run_instruction(uint8_t opcode) {
    switch (opcode) {
        case 0xCA: { // DEX
            --IND_REG_X;
            SET_Z(IND_REG_X == 0); // TODO: find out if Z should be unset too or just set on zero
            SET_N(IS_NEG(IND_REG_X));
            break;
        }
        case 0xE8: { // INX
            ++IND_REG_X;
            SET_Z(IND_REG_X == 0);
            SET_N(IS_NEG(IND_REG_X));
            break;
        }
        case 0x88: { // DEY
            --IND_REG_Y;
            SET_Z(IND_REG_Y == 0);
            SET_N(IS_NEG(IND_REG_Y));
            break;
        }
        case 0xC8: { // INY
            ++IND_REG_Y;
            SET_Z(IND_REG_Y == 0);
            SET_N(IS_NEG(IND_REG_Y));
            break;
        }
        case 0x38: { // SEC
            SET_C(1);
            break;
        }
        case 0xF8: { // SED
            SET_D(1);
            break;
        }
        case 0x78: { // SEI
            SET_I(1);
            break;
        }
        case 0xAA: { // TAX
            IND_REG_X = ACCUMULATOR;
            SET_Z(IND_REG_X == 0);
            SET_N(IS_NEG(IND_REG_X));
            break;
        }
        case 0xA8: { // TAY
            IND_REG_Y = ACCUMULATOR;
            SET_Z(IND_REG_Y == 0);
            SET_N(IS_NEG(IND_REG_Y));
            break;
        }
        case 0xBA: { // TSX
            IND_REG_X = STACK_POINTER;
            SET_Z(IND_REG_X == 0);
            SET_N(IS_NEG(IND_REG_X));
            break;
        }
        case 0x8A: { // TXA
            ACCUMULATOR = IND_REG_X;
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0x9A: { // TXS
            STACK_POINTER = IND_REG_X;
            break;
        }
        case 0x98: { // TYA
            ACCUMULATOR = IND_REG_Y;
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xEA: { // NOP
            break;
        }
    }
    ++PC;
}

void reset() {
    PC = 0;
    STACK_POINTER = 0;
    ACCUMULATOR = 0;
    IND_REG_X = 0;
    IND_REG_Y = 0;
    STATUS_REG = 0;
    for (uint8_t i = 0; i < 0xFF; ++i) {
        MEMORY[i] = 0;
    }
}


int main(int argc, char *argv[]) {
    std::printf("nes emulator");

    // DEX
    reset();
    IND_REG_X = 2;
    run_instruction(0xCA);
    if (IND_REG_X != 1 && !GET_Z() && !GET_N()) std::printf("DEX\n");
    run_instruction(0xCA);
    if (IND_REG_X != 0 && GET_Z() && !GET_N()) std::printf("DEX zero\n");
    run_instruction(0xCA);
    if (IND_REG_X != 0xFF && !GET_Z() && GET_N()) std::printf("DEX neg\n");
}
