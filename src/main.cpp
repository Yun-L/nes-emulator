#include <cstdio>
#include <cstdint>

#define IS_NEG(uint8_num) (1 == uint8_num >> 7)
#define SET_C(b) ((STATUS_REG &= 0b11111110) | (uint8_t)(b))
#define SET_Z(b) ((STATUS_REG &= 0b11111101) | ((uint8_t)(b) << 1))
#define SET_I(b) ((STATUS_REG &= 0b11111011) | ((uint8_t)(b) << 2))
#define SET_D(b) ((STATUS_REG &= 0b11110111) | ((uint8_t)(b) << 3))
#define SET_V(b) ((STATUS_REG &= 0b10111111) | ((uint8_t)(b) << 6))
#define SET_N(b) ((STATUS_REG &= 0b01111111) | ((uint8_t)(b) << 7))
#define GET_C() ((STATUS_REG & 0b00000001) > 0)
#define GET_Z() ((STATUS_REG & 0b00000010) > 0)
#define GET_I() ((STATUS_REG & 0b00000100) > 0)
#define GET_D() ((STATUS_REG & 0b00001000) > 0)
#define GET_V() ((STATUS_REG & 0b01000000) > 0)
#define GET_N() ((STATUS_REG & 0b10000000) > 0)

uint16_t PC;
uint8_t STACK_POINTER; //stack is 0x0100 - 0x01FF in memory, pointer is least significant half
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

uint8_t* addr_immediate() {
    ++PC;
    return &(MEMORY[PC]);
}

uint8_t* addr_zero_page() {
    ++PC;
    uint16_t addr = (uint16_t)MEMORY[PC];
    return &(MEMORY[addr]);
}

uint8_t* addr_zero_page_x() {
    ++PC;
    uint16_t addr = (uint16_t)MEMORY[PC];
    addr = (addr + IND_REG_X) & 0x00FFu; // zero page wrap around
    return &(MEMORY[addr]);
}

uint8_t* addr_zero_page_y() {
    ++PC;
    uint16_t addr = (uint16_t)MEMORY[PC];
    addr = (addr + IND_REG_Y) & 0x00FFu; // zero page wrap around
    return &(MEMORY[addr]);
}

uint8_t* addr_abs() {
    PC += 2;
    uint16_t addr = (uint16_t)(((uint16_t)MEMORY[PC] << 8) | (uint16_t)MEMORY[PC - 1]);
    return &(MEMORY[addr]);
}

uint8_t* addr_abs_x() {
    PC += 2;
    uint16_t addr = (uint16_t)(((uint16_t)MEMORY[PC] << 8) | (uint16_t)MEMORY[PC - 1]);
    addr += IND_REG_X; // allow 16 bit wrap around
    return &(MEMORY[addr]);
}

uint8_t* addr_abs_y() {
    PC += 2;
    uint16_t addr = (uint16_t)(((uint16_t)MEMORY[PC] << 8) | (uint16_t)MEMORY[PC - 1]);
    addr += IND_REG_Y; // allow 16 bit wrap around
    return &(MEMORY[addr]);
}

uint8_t* addr_indexed_indirect() {
    ++PC;
    uint8_t pt = (uint8_t)(MEMORY[PC] + IND_REG_X); // allow zero page wrap around
    // note: full addr should always be in zero page, meaning pt + 1 should NOT
    //       cross the zero page boundary
    // TODO: figure out what to do if it does cross the boundary
    uint16_t addr = (uint16_t)(((uint16_t)MEMORY[pt + 1] << 8) | (uint16_t)MEMORY[pt]);
    return &(MEMORY[addr]);
}

uint8_t* addr_indirect_indexed() {
    ++PC;
    uint16_t pt = (uint16_t)MEMORY[MEMORY[PC]];
    uint16_t addr = (uint16_t)((((uint16_t)MEMORY[pt + 1] << 8) | (uint16_t)MEMORY[pt]) + IND_REG_Y);
    return &(MEMORY[addr]);
}

uint8_t* addr_accumulator() {
    return &ACCUMULATOR;
}


void ADC(uint8_t* pt) {
    uint8_t sign_a = (uint8_t)(ACCUMULATOR >> 7);
    uint8_t sign_m = (uint8_t)(MEMORY[PC] >> 7);
    SET_C((ACCUMULATOR + MEMORY[PC]) > 0xFFu);
    ACCUMULATOR += *pt;
    uint8_t sign_a_fin = (uint8_t)(ACCUMULATOR >> 7);
    SET_Z(ACCUMULATOR == 0);
    if (sign_a == sign_m) {
        SET_V(sign_a_fin != sign_a);
    }
    SET_N(sign_a_fin & 1);
}

void AND(uint8_t* pt) {
    ACCUMULATOR &= *pt;
    SET_Z(ACCUMULATOR == 0);
    SET_N((ACCUMULATOR >> 7) & 1);
}

void ASL(uint8_t* pt) {
    SET_C((*pt >> 7) & 1);
    *pt = (uint8_t)(*pt << 1);
    SET_Z(*pt == 0);
    SET_N((*pt >> 7) & 1);
}

void BCC(uint8_t* pt) {
    if (!GET_C()) PC += (int8_t)(*pt);
}

void BCS(uint8_t* pt) {
    if (GET_C()) PC += (int8_t)(*pt);
}

void BEQ(uint8_t* pt) {
    if (GET_Z()) PC += (int8_t)(*pt);
}

void BIT(uint8_t* pt) {
    uint8_t tmp = (uint8_t)(*pt & ACCUMULATOR);
    SET_Z(tmp == 0);
    SET_V((tmp >> 6) & 1);
    SET_N((tmp >> 7) & 1);
}

void BMI(uint8_t* pt) {
    if (GET_N()) PC += (int8_t)(*pt);
}

void BNE(uint8_t* pt) {
    if (!GET_Z()) PC += (int8_t)(*pt);
}

void BPL(uint8_t* pt) {
    if (!GET_N()) PC += (int8_t)(*pt);
}

void BRK() {}

void BVC(uint8_t* pt) {
    if (!GET_V()) PC += (int8_t)(*pt);
}

void BVS(uint8_t* pt) {
    if (GET_V()) PC += (int8_t)(*pt);
}

void CLC() {
    SET_C(0);
}

void CLD() {
    SET_D(0);
}

void CLI() {
    SET_I(0);
}

void CLV() {
    SET_V(0);
}

void CMP(uint8_t* pt) {
    uint8_t val = *pt;
    SET_C(ACCUMULATOR >= val);
    SET_Z(ACCUMULATOR == val);
    SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
}

void CPX(uint8_t* pt) {
    uint8_t val = *pt;
    SET_C(IND_REG_X >= val);
    SET_Z(IND_REG_X == val);
    SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
}

void CPY(uint8_t* pt) {
    uint8_t val = *pt;
    SET_C(IND_REG_Y >= val);
    SET_Z(IND_REG_Y == val);
    SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
}

void DEC(uint8_t* pt) {
    *pt -= 1; // TODO: what should happen at 0?
    uint8_t val = *pt;
    SET_Z(val == 0); // TODO: find out if Z should be unset too or just set on zero
    SET_N(IS_NEG(val));
}

void DEX() {
    --IND_REG_X;
    SET_Z(IND_REG_X == 0); // TODO: find out if Z should be unset too or just set on zero
    SET_N(IS_NEG(IND_REG_X));
}

void DEY() {
    --IND_REG_Y;
    SET_Z(IND_REG_Y == 0);
    SET_N(IS_NEG(IND_REG_Y));
}

void EOR(uint8_t* pt) {
    uint8_t val = *pt;
    ACCUMULATOR = (uint8_t)(ACCUMULATOR ^ val);
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void INC(uint8_t* pt) {
    ++(*pt);
    SET_Z(*pt == 0);
    SET_N(IS_NEG(*pt));
}

void INX() {
    ++IND_REG_X;
    SET_Z(IND_REG_X == 0);
    SET_N(IS_NEG(IND_REG_X));
}

void INY() {
    ++IND_REG_Y;
    SET_Z(IND_REG_Y == 0);
    SET_N(IS_NEG(IND_REG_Y));
}

void JMP(uint8_t* pt) {}

void JSR(uint8_t* pt) {
    STACK_POINTER -= 2;
    MEMORY[STACK_POINTER + 2] = (uint8_t)((PC & 0xFF00) >> 8);
    MEMORY[STACK_POINTER + 1] = (uint8_t)(PC & 0xFF);
    // PC = addr;
    // TODO: fix this
}

void LDA(uint8_t* pt) {
    ACCUMULATOR = *pt;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void LDX(uint8_t* pt) {
    IND_REG_X = *pt;
    SET_Z(IND_REG_X);
    SET_N(IS_NEG(IND_REG_X));
}

void LDY(uint8_t* pt) {
    IND_REG_Y = *pt;
    SET_Z(IND_REG_Y);
    SET_N(IS_NEG(IND_REG_Y));
}

void LSR(uint8_t* pt) {
    SET_C(*pt & 0x01);
    *pt = (uint8_t)(*pt >> 1);
    SET_Z(*pt == 0);
    SET_N(false); // high bit is always 0 after shift
}


void NOP() {}

void ORA(uint8_t* pt) {}

void PHA() {}

void PHP() {}

void PLA() {}

void PLP() {}

void ROL(uint8_t* pt) {}

void ROR(uint8_t* pt) {}

void RTI() {}

void RTS() {}

void SBC(uint8_t* pt) {}

void SEC() {
    SET_C(1);
}

void SED() {
    SET_D(1);
}

void SEI() {
    SET_I(1);
}

void STA(uint8_t* pt) {}

void STX(uint8_t* pt) {}

void STY(uint8_t* pt) {}

void TAX() {
    IND_REG_X = ACCUMULATOR;
    SET_Z(IND_REG_X == 0);
    SET_N(IS_NEG(IND_REG_X));
}

void TAY() {
    IND_REG_Y = ACCUMULATOR;
    SET_Z(IND_REG_Y == 0);
    SET_N(IS_NEG(IND_REG_Y));
}

void TSX() {
    IND_REG_X = STACK_POINTER;
    SET_Z(IND_REG_X == 0);
    SET_N(IS_NEG(IND_REG_X));
}

void TXA() {
    ACCUMULATOR = IND_REG_X;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void TXS() {
    STACK_POINTER = IND_REG_X;
}

void TYA() {
    ACCUMULATOR = IND_REG_Y;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void run_instruction(uint8_t opcode) {
    uint8_t* pt;
    switch (opcode) {
        // Load/Store Operations
        case 0xA9: pt = addr_immediate();        LDA(pt); break;
        case 0xA5: pt = addr_zero_page();        LDA(pt); break;
        case 0xB5: pt = addr_zero_page_x();      LDA(pt); break;
        case 0xAD: pt = addr_abs();              LDA(pt); break;
        case 0xBD: pt = addr_abs_x();            LDA(pt); break;
        case 0xB9: pt = addr_abs_y();            LDA(pt); break;
        case 0xA1: pt = addr_indexed_indirect(); LDA(pt); break;
        case 0xB1: pt = addr_indirect_indexed(); LDA(pt); break;
        case 0xA2: pt = addr_immediate();        LDX(pt); break;
        case 0xA6: pt = addr_zero_page();        LDX(pt); break;
        case 0xB6: pt = addr_zero_page_y();      LDX(pt); break;
        case 0xAE: pt = addr_abs();              LDX(pt); break;
        case 0xBE: pt = addr_abs_y();            LDX(pt); break;
        case 0xA0: pt = addr_immediate();        LDY(pt); break;
        case 0xA4: pt = addr_zero_page();        LDY(pt); break;
        case 0xB4: pt = addr_zero_page_x();      LDY(pt); break;
        case 0xAC: pt = addr_abs();              LDY(pt); break;
        case 0xBC: pt = addr_abs_x();            LDY(pt); break;
        case 0x85: pt = addr_zero_page();        STA(pt); break;
        case 0x95: pt = addr_zero_page_x();      STA(pt); break;
        case 0x8D: pt = addr_abs();              STA(pt); break;
        case 0x9D: pt = addr_abs_x();            STA(pt); break;
        case 0x99: pt = addr_abs_y();            STA(pt); break;
        case 0x81: pt = addr_indexed_indirect(); STA(pt); break;
        case 0x91: pt = addr_indirect_indexed(); STA(pt); break;
        case 0x86: pt = addr_zero_page();        STX(pt); break;
        case 0x96: pt = addr_zero_page_y();      STX(pt); break;
        case 0x8E: pt = addr_abs();              STX(pt); break;
        case 0x84: pt = addr_zero_page();        STY(pt); break;
        case 0x94: pt = addr_zero_page_x();      STY(pt); break;
        case 0x8C: pt = addr_abs();              STY(pt); break;

        // Register Transfers
        case 0xAA: /* implied */                 TAX(); break;
        case 0xA8: /* implied */                 TAY(); break;
        case 0x8A: /* implied */                 TXA(); break;
        case 0x98: /* implied */                 TYA(); break;

        // Stack Operation
        case 0xBA: /* implied */                 TSX(); break;
        case 0x9A: /* implied */                 TXS(); break;
        case 0x48: /* implied */                 PHA(); break;
        case 0x08: /* implied */                 PHP(); break;
        case 0x68: /* implied */                 PLA(); break;
        case 0x28: /* implied */                 PLP(); break;

        // Logical
        case 0x29: pt = addr_immediate();        AND(pt); break;
        case 0x25: pt = addr_zero_page();        AND(pt); break;
        case 0x35: pt = addr_zero_page_x();      AND(pt); break;
        case 0x2D: pt = addr_abs();              AND(pt); break;
        case 0x3D: pt = addr_abs_x();            AND(pt); break;
        case 0x39: pt = addr_abs_y();            AND(pt); break;
        case 0x21: pt = addr_indexed_indirect(); AND(pt); break;
        case 0x31: pt = addr_indirect_indexed(); AND(pt); break;
        case 0x49: pt = addr_immediate();        EOR(pt); break;
        case 0x45: pt = addr_zero_page();        EOR(pt); break;
        case 0x55: pt = addr_zero_page_x();      EOR(pt); break;
        case 0x4D: pt = addr_abs();              EOR(pt); break;
        case 0x5D: pt = addr_abs_x();            EOR(pt); break;
        case 0x59: pt = addr_abs_y();            EOR(pt); break;
        case 0x41: pt = addr_indexed_indirect(); EOR(pt); break;
        case 0x51: pt = addr_indirect_indexed(); EOR(pt); break;
        case 0x09: pt = addr_immediate();        ORA(pt); break;
        case 0x05: pt = addr_zero_page();        ORA(pt); break;
        case 0x15: pt = addr_zero_page_x();      ORA(pt); break;
        case 0x0D: pt = addr_abs();              ORA(pt); break;
        case 0x1D: pt = addr_abs_x();            ORA(pt); break;
        case 0x19: pt = addr_abs_y();            ORA(pt); break;
        case 0x01: pt = addr_indexed_indirect(); ORA(pt); break;
        case 0x11: pt = addr_indirect_indexed(); ORA(pt); break;
        case 0x24: pt = addr_zero_page();        BIT(pt); break;
        case 0x2C: pt = addr_abs();              BIT(pt); break;

        // Arithmetic
        case 0x69: pt = addr_immediate();        ADC(pt); break;
        case 0x65: pt = addr_zero_page();        ADC(pt); break;
        case 0x75: pt = addr_zero_page_x();      ADC(pt); break;
        case 0x6D: pt = addr_abs();              ADC(pt); break;
        case 0x7D: pt = addr_abs_x();            ADC(pt); break;
        case 0x79: pt = addr_abs_y();            ADC(pt); break;
        case 0x61: pt = addr_indexed_indirect(); ADC(pt); break;
        case 0x71: pt = addr_indirect_indexed(); ADC(pt); break;
        case 0xE9: pt = addr_immediate();        SBC(pt); break;
        case 0xE5: pt = addr_zero_page();        SBC(pt); break;
        case 0xF5: pt = addr_zero_page_x();      SBC(pt); break;
        case 0xED: pt = addr_abs();              SBC(pt); break;
        case 0xFD: pt = addr_abs_x();            SBC(pt); break;
        case 0xF9: pt = addr_abs_y();            SBC(pt); break;
        case 0xE1: pt = addr_indexed_indirect(); SBC(pt); break;
        case 0xF1: pt = addr_indirect_indexed(); SBC(pt); break;
        case 0xC9: pt = addr_immediate();        CMP(pt); break;
        case 0xC5: pt = addr_zero_page();        CMP(pt); break;
        case 0xD5: pt = addr_zero_page_x();      CMP(pt); break;
        case 0xCD: pt = addr_abs();              CMP(pt); break;
        case 0xDD: pt = addr_abs_x();            CMP(pt); break;
        case 0xD9: pt = addr_abs_y();            CMP(pt); break;
        case 0xC1: pt = addr_indexed_indirect(); CMP(pt); break;
        case 0xD1: pt = addr_indirect_indexed(); CMP(pt); break;
        case 0xE0: pt = addr_immediate();        CPX(pt); break;
        case 0xE4: pt = addr_zero_page();        CPX(pt); break;
        case 0xEC: pt = addr_abs();              CPX(pt); break;
        case 0xC0: pt = addr_immediate();        CPY(pt); break;
        case 0xC4: pt = addr_zero_page();        CPY(pt); break;
        case 0xCC: pt = addr_abs();              CPY(pt); break;

        // Increments & Decrements
        case 0xE8: /* implied */                 INX(); break;
        case 0xC8: /* implied */                 INY(); break;
        case 0xCA: /* implied */                 DEX(); break;
        case 0x88: /* implied */                 DEY(); break;
        case 0xE6: pt = addr_zero_page();        INC(pt); break;
        case 0xF6: pt = addr_zero_page_x();      INC(pt); break;
        case 0xEE: pt = addr_abs();              INC(pt); break;
        case 0xFE: pt = addr_abs_x();            INC(pt); break;
        case 0xC6: pt = addr_zero_page();        DEC(pt); break;
        case 0xD6: pt = addr_zero_page_x();      DEC(pt); break;
        case 0xCE: pt = addr_abs();              DEC(pt); break;
        case 0xDE: pt = addr_abs_x();            DEC(pt); break;

        // Shifts
        case 0x0A: pt = addr_accumulator();      ASL(pt); break;
        case 0x06: pt = addr_zero_page();        ASL(pt); break;
        case 0x16: pt = addr_zero_page_x();      ASL(pt); break;
        case 0x0E: pt = addr_abs();              ASL(pt); break;
        case 0x1E: pt = addr_abs_x();            ASL(pt); break;
        case 0x4A: pt = addr_accumulator();      LSR(pt); break;
        case 0x46: pt = addr_zero_page();        LSR(pt); break;
        case 0x56: pt = addr_zero_page_x();      LSR(pt); break;
        case 0x4E: pt = addr_abs();              LSR(pt); break;
        case 0x5E: pt = addr_abs_x();            LSR(pt); break;
        case 0x2A: pt = addr_accumulator();      ROL(pt); break;
        case 0x26: pt = addr_zero_page();        ROL(pt); break;
        case 0x36: pt = addr_zero_page_x();      ROL(pt); break;
        case 0x2E: pt = addr_abs();              ROL(pt); break;
        case 0x3E: pt = addr_abs_x();            ROL(pt); break;
        case 0x6A: pt = addr_accumulator();      ROR(pt); break;
        case 0x66: pt = addr_zero_page();        ROR(pt); break;
        case 0x76: pt = addr_zero_page_x();      ROR(pt); break;
        case 0x6E: pt = addr_abs();              ROR(pt); break;
        case 0x7E: pt = addr_abs_x();            ROR(pt); break;

        // Jumps & Calls
        case 0x4C: pt = addr_abs();              JMP(pt); break;
        case 0x6C: pt = 0; /* indirect */        JMP(pt); break;
        case 0x20: pt = addr_abs();              JSR(pt); break;
        case 0x60: /* implied */                 RTS();   break;

        // Branches
        case 0x90: pt = addr_immediate();        BCC(pt); break;
        case 0x80: pt = addr_immediate();        BCS(pt); break;
        case 0xF0: pt = addr_immediate();        BEQ(pt); break;
        case 0x30: pt = addr_immediate();        BMI(pt); break;
        case 0xD0: pt = addr_immediate();        BNE(pt); break;
        case 0x10: pt = addr_immediate();        BPL(pt); break;
        case 0x50: pt = addr_immediate();        BVC(pt); break;
        case 0x70: pt = addr_immediate();        BVS(pt); break;

        // Status Flag Changes
        case 0x18: /* implied */                 CLC();  break;
        case 0xD8: /* implied */                 CLD();  break;
        case 0x58: /* implied */                 CLI();  break;
        case 0xB8: /* implied */                 CLV();  break;
        case 0x38: /* implied */                 SEC();  break;
        case 0xF8: /* implied */                 SED();  break;
        case 0x78: /* implied */                 SEI();  break;

        // System Functions
        case 0x00: /* implied */                 BRK();  break;
        case 0xEA: /* implied */                 NOP();  break;
        case 0x40: /* implied */                 RTI();  break;

        default:
            // note: illegal or undocumented opcode
            // decide on whether or not we want to implement the undocumented ones
            // not sure if any ROMS use those
            break;
    }
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


int main() {
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
