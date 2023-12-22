#include <cstdio>
#include <cstdint>

#define IS_NEG(uint8_num) (1 == uint8_num >> 7)
#define SET_C(b) ((STATUS_REG &= 11111110) | (bool)b)
#define SET_Z(b) ((STATUS_REG &= 11111101) | ((bool)b << 1))
#define SET_I(b) ((STATUS_REG &= 11111011) | ((bool)b << 2))
#define SET_D(b) ((STATUS_REG &= 11110111) | ((bool)b << 3))
#define SET_V(b) ((STATUS_REG &= 10111111) | ((bool)b << 6))
#define SET_N(b) ((STATUS_REG &= 01111111) | ((bool)b << 7))
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
        case 0x69: { // ADC Immediate
            ++PC;
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = MEMORY[PC] >> 7;
            SET_C(((uint_16)ACCUMULATOR + MEMORY[PC]) > (uint8_t)0xFF);
            ACCUMULATOR += MEMORY[PC];
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x65: { // ADC ZP
            ++PC;
            uint8_t addr = MEMORY[PC];
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = MEMORY[addr] >> 7;
            SET_C(((uint_16)ACCUMULATOR + MEMORY[addr]) > (uint8_t)0xFF);
            ACCUMULATOR += MEMORY[addr];
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x75: { // ADC ZP,X
            ++PC;
            uint8_t addr = MEMORY[PC];
            uint8_t m_val = MEMORY[addr + IND_REG_X];
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = m_val >> 7;
            SET_C(((uint_16)ACCUMULATOR + m_val) > (uint8_t)0xFF);
            ACCUMULATOR += m_val;
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x6D: { // ADC Abs
            PC += 2;
            uint16_t addr = (MEMORY[PC] << 8) + MEMORY[PC-1];
            uint8_t m_val = MEMORY[addr];
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = m_val >> 7;
            SET_C(((uint_16)ACCUMULATOR + m_val) > (uint8_t)0xFF);
            ACCUMULATOR += m_val;
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x7D: { // ADC Abs,X
            PC += 2;
            uint16_t addr = (MEMORY[PC] << 8) + MEMORY[PC-1] + IND_REG_X;
            uint8_t m_val = MEMORY[addr];
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = m_val >> 7;
            SET_C(((uint_16)ACCUMULATOR + m_val) > (uint8_t)0xFF);
            ACCUMULATOR += m_val;
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x79: { // ADC Abs,Y
            PC += 2;
            uint16_t addr = (MEMORY[PC] << 8) + MEMORY[PC-1] + IND_REG_Y;
            uint8_t m_val = MEMORY[addr];
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = m_val >> 7;
            SET_C(((uint_16)ACCUMULATOR + m_val) > (uint8_t)0xFF);
            ACCUMULATOR += m_val;
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x61: { // ADC Ind,X
            ++PC;
            uint8_t pt = MEMORY[PC];
            uint8_t ptx = MEMORY[pt] + IND_REG_X;
            uint16_t addr = (MEMORY[ptx + 1] << 8) + MEMORY[ptx];
            uint8_t m_val = MEMORY[addr];
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = m_val >> 7;
            SET_C(((uint_16)ACCUMULATOR + m_val) > (uint8_t)0xFF);
            ACCUMULATOR += m_val;
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x71: { // ADC Ind,Y
            ++PC;
            uint8_t pt = MEMORY[PC];
            uint16_t eff_l = MEMORY[pt] + IND_REG_Y;
            uint8_t eff_h = MEMORY[pt + 1];
            // carry over to high byte if needed
            // TODO: will need extra cycle to fix high byte
            eff_h += (eff_l >> 8) & 0x00FF;
            uint16_t addr = ((eff_h << 8) & 0xFF00) | (eff_l & 0x00FF);
            uint8_t m_val = MEMORY[addr];
            uint8_t sign_a = ACCUMULATOR >> 7;
            uint8_t sign_m = m_val >> 7;
            SET_C(((uint_16)ACCUMULATOR + m_val) > (uint8_t)0xFF);
            ACCUMULATOR += m_val;
            uint8_t sign_a_fin = ACCUMULATOR >> 7;
            SET_Z(ACCUMULATOR == 0);
            if (sign_a == sign_m) {
                SET_V(sign_a_fin != sign_a);
            }
            SET_N(sign_a_fin & 1);
            break;
        }
        case 0x24: { // BIT ZP
            ++PC;
            uint8_t addr = MEMORY[PC];
            uint8_t tmp = MEMORY[addr] & ACCUMULATOR;
            SET_Z(tmp == 0);
            SET_V((tmp >> 6) & 1);
            SET_N((tmp >> 7) & 1);
            break;
        }
        case 0x2C: { // BIT Abs
            PC += 2;
            uint16_t addr = (MEMORY[PC] << 8) + MEMORY[PC-1];
            uint8_t tmp = MEMORY[addr] & ACCUMULATOR;
            SET_Z(tmp == 0);
            SET_V((tmp >> 6) & 1);
            SET_N((tmp >> 7) & 1);
            break;
        }
        case 0x18: { //CLC
            SET_C(0);
            break;
        }
        case 0xD8: { //CLD
            SET_D(0);
            break;
        }
        case 0x58: { //CLI
            SET_I(0);
            break;
        }
        case 0xB8: { //CLV
            SET_V(0);
            break;
        }
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
        case 0xA9: { // LDA Imm
            ++PC;
            ACCUMULATOR = MEMORY[PC];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xA5: { // LDA ZP
            ++PC;
            uint8_t addr = MEMORY[PC];
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xB5: { // LDA ZPX
            ++PC;
            uint8_t addr = MEMORY[PC];
            ACCUMULATOR = MEMORY[addr] + IND_REG_X;
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xAD: { // LDA Abs
            PC += 2;
            uint16_t addr = (MEMORY[PC] << 8) + MEMORY[PC - 1];
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xBD: { // LDA AbsX
            PC += 2;
            uint16_t addr = (MEMORY[PC] << 8) + MEMORY[PC - 1];
            ACCUMULATOR = MEMORY[addr] + IND_REG_X;
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xB9: { // LDA AbsY
            PC += 2;
            uint16_t addr = (MEMORY[PC] << 8) + MEMORY[PC - 1];
            ACCUMULATOR = MEMORY[addr] + IND_REG_Y;
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xA1: { // LDA IndX
            ++PC;
            uint8_t pt = MEMORY[PC];
            uint8_t ptx = MEMORY[pt] + IND_REG_X;
            uint16_t addr = (MEMORY[ptx + 1] << 8) + MEMORY[ptx];
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xB1: { // LDA IndY
            ++PC;
            uint8_t pt = MEMORY[PC];
            uint16_t addr = (MEMORY[pt + 1] << 8) + MEMORY[pt] + IND_REG_Y;
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
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
