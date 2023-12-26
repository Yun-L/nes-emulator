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
uint8_t addr_zero_page() {
    ++PC;
    return MEMORY[PC];
}

uint8_t addr_zero_page_x() {
    return addr_zero_page() + IND_REG_X; // zero page wrap around
}

uint8_t addr_zero_page_y() {
    return addr_zero_page() + IND_REG_Y; // zero page wrap around
}

uint16_t addr_abs() {
    PC += 2;
    return ((MEMORY[PC] << 8) & 0xFF00) | (MEMORY[PC - 1] & 0x00FF);
}

uint16_t addr_abs_x() {
    return addr_abs() + IND_REG_X;
}

uint16_t addr_abs_y() {
    return addr_abs() + IND_REG_Y;
}

uint16_t addr_indexed_indirect() {
    ++PC;
    uint8_t pt = MEMORY[PC];
    uint8_t ptx = MEMORY[pt] + IND_REG_X; // zero page wrap around
    return ((MEMORY[ptx + 1] << 8) & 0xFF00) | (MEMORY[ptx] & 0x00FF);
}

uint16_t addr_indirect_indexed() {
    ++PC;
    uint8_t pt = MEMORY[PC];
    uint16_t eff_l = MEMORY[pt] + IND_REG_Y;
    uint8_t eff_h = MEMORY[pt + 1];
    // carry over to high byte if needed
    // TODO: will need extra cycle to fix high byte
    eff_h += (eff_l >> 8) & 0x00FF;
    return ((eff_h << 8) & 0xFF00) | (eff_l & 0x00FF);
}

uint8_t addr_relative() {
    ++PC;
    uint8_t operand = MEMORY[PC];
    return operand;
}

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
            uint8_t addr = addr_zero_page();
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
            uint8_t addr = addr_zero_page_x();
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
        case 0x6D: { // ADC Abs
            uint16_t addr = addr_abs();
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
            uint16_t addr = addr_abs_x();
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
            uint16_t addr = addr_abs_y();
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
        case 0x61: { // ADC (Ind,X)
            uint16_t addr = addr_indexed_indirect();
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
        case 0x71: { // ADC (Ind),Y
            uint16_t addr = addr_indirect_indexed();
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
        case 0x29: { // AND Immediate
            ++PC;
            ACCUMULATOR &= MEMORY[PC];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x25: { // AND ZP
            uint8_t addr = addr_zero_page();
            ACCUMULATOR &= MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x35: { // AND ZP,X
            uint8_t addr = addr_zero_page_x();
            ACCUMULATOR &= MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x2D: { // AND Abs
            uint16_t addr = addr_abs();
            ACCUMULATOR &= MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x3D: { // AND Abs,X
            uint16_t addr = addr_abs_x();
            ACCUMULATOR &= MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x39: { // AND Abs,Y
            uint16_t addr = addr_abs_y();
            ACCUMULATOR &= MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x21: { // AND (Ind,X)
            uint16_t addr = addr_indexed_indirect();
            ACCUMULATOR &= MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x31: { // AND (Ind),Y
            uint16_t addr = addr_indirect_indexed();
            ACCUMULATOR &= MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x0A: { // ASL A
            SET_C((ACCUMULATOR >> 7) & 1);
            ACCUMULATOR = ACCUMULATOR << 1;
            SET_Z(ACCUMULATOR == 0);
            SET_N((ACCUMULATOR >> 7) & 1);
            break;
        }
        case 0x06: { // ASL ZP
            uint8_t addr = addr_zero_page();
            SET_C((MEMORY[addr] >> 7) & 1);
            MEMORY[addr] = MEMORY[addr] << 1;
            SET_Z(MEMORY[addr] == 0);
            SET_N((MEMORY[addr] >> 7) & 1);
            break;
        }
        case 0x16: { // ASL ZP,X
            uint8_t addr = addr_zero_page_x();
            SET_C((MEMORY[addr] >> 7) & 1);
            MEMORY[addr] = MEMORY[addr] << 1;
            SET_Z(MEMORY[addr] == 0);
            SET_N((MEMORY[addr] >> 7) & 1);
            break;
        }
        case 0x0E: { // ASL Abs
            uint16_t addr = addr_abs();
            SET_C((MEMORY[addr] >> 7) & 1);
            MEMORY[addr] = MEMORY[addr] << 1;
            SET_Z(MEMORY[addr] == 0);
            SET_N((MEMORY[addr] >> 7) & 1);
            break;
        }
        case 0x1E: { // ASL Abs,X
            uint16_t addr = addr_abs_x();
            SET_C((MEMORY[addr] >> 7) & 1);
            MEMORY[addr] = MEMORY[addr] << 1;
            SET_Z(MEMORY[addr] == 0);
            SET_N((MEMORY[addr] >> 7) & 1);
            break;
        }
        case 0xB0: { // BCS
            if (GET_C()) {
                PC += addr_relative(); // TODO: should it be addr-1?
            }
            break;
        }
        case 0xF0: { // BEQ
            if (GET_Z()) {
                PC += addr_relative(); // TODO: see BCS
            }
            break;
        }
        case 0x24: { // BIT ZP
            uint8_t addr = addr_zero_page();
            uint8_t tmp = MEMORY[addr] & ACCUMULATOR;
            SET_Z(tmp == 0);
            SET_V((tmp >> 6) & 1);
            SET_N((tmp >> 7) & 1);
            break;
        }
        case 0x2C: { // BIT Abs
            uint16_t addr = addr_abs();
            uint8_t tmp = MEMORY[addr] & ACCUMULATOR;
            SET_Z(tmp == 0);
            SET_V((tmp >> 6) & 1);
            SET_N((tmp >> 7) & 1);
            break;
        }
        case 0x30: { // BMI
            if (GET_N()) {
                PC += addr_relative(); // TODO: see BCS
            }
            break;
        }
        case 0xD0: { // BNE
            if (!GET_Z()) {
                PC += addr_relative(); // TODO: see BCS
            }
            break;
        }
        case 0x10: { // BPL
            if (!GET_N()) {
                PC += addr_relative(); // TODO: see BCS
            }
            break;
        }
        case 0x00: { // BRK
            // TODO: not sure how to handle this rn
            break;
        }
        case 0x50: { // BVC
            if (!GET_V()) {
                PC += addr_relative(); // TODO: see BCS
            }
            break;
        }
        case 0x70: { // BVS
            if (GET_V()) {
                PC += addr_relative(); // TODO: see BCS
            }
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
        case 0xC9: { // CMP Imm
            ++PC;
            uint8_t val = MEMORY[PC];
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xC5: { // CMP ZP
            uint8_t val = MEMORY[addr_zero_page());
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xD5: { // CMP ZP,X
            uint8_t val = MEMORY[addr_zero_page_x());
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xCD: { // CMP Abs
            uint8_t val = MEMORY[addr_abs());
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xDD: { // CMP Abs, X
            uint8_t val = MEMORY[addr_abs_x());
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xD9: { // CMP Abs, Y
            uint8_t val = MEMORY[addr_abs_y());
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xC1: { // CMP (Ind,X)
            uint8_t val = MEMORY[addr_indexed_indirect());
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xD1: { // CMP (Ind),Y
            uint8_t val = MEMORY[addr_indirect_indexed());
            SET_C(ACCUMULATOR >= val);
            SET_Z(ACCUMULATOR == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xE0: { // CPX Imm
            ++PC;
            uint8_t val = MEMORY[PC];
            SET_C(IND_REG_X >= val);
            SET_Z(IND_REG_X == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xE4: { // CPX ZP
            uint8_t val = MEMORY[addr_zero_page()];
            SET_C(IND_REG_X >= val);
            SET_Z(IND_REG_X == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xEC: { // CPX Abs
            uint8_t val = MEMORY[addr_abs()];
            SET_C(IND_REG_X >= val);
            SET_Z(IND_REG_X == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xC0: { // CPY Imm
            ++PC;
            uint8_t val = MEMORY[PC];
            SET_C(IND_REG_Y >= val);
            SET_Z(IND_REG_Y == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xC4: { // CPY ZP
            uint8_t val = MEMORY[addr_zero_page()];
            SET_C(IND_REG_Y >= val);
            SET_Z(IND_REG_Y == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
            break;
        }
        case 0xCC: { // CPY Abs
            uint8_t val = MEMORY[addr_abs()];
            SET_C(IND_REG_Y >= val);
            SET_Z(IND_REG_Y == val);
            SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
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
            uint8_t addr = addr_zero_page();
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
            uint16_t addr = addr_abs();
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xBD: { // LDA AbsX
            uint16_t addr = addr_abs_x();
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xB9: { // LDA AbsY
            uint16_t addr = addr_abs_y();
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xA1: { // LDA (Ind,X)
            uint16_t addr = addr_indexed_indirect();
            ACCUMULATOR = MEMORY[addr];
            SET_Z(ACCUMULATOR == 0);
            SET_N(IS_NEG(ACCUMULATOR));
            break;
        }
        case 0xB1: { // LDA (Ind),Y
            uint16_t addr = addr_indirect_indexed();
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
