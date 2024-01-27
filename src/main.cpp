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

// returns a sign byte, represents the offset for a relative address
int8_t addr_relative() {
    ++PC;
    return (int8_t)MEMORY[PC];
}

void ADC(uint16_t addr) {
    uint8_t sign_a = ACCUMULATOR >> 7;
    uint8_t sign_m = MEMORY[PC] >> 7;
    SET_C(((uint_16)ACCUMULATOR + MEMORY[PC]) > (uint8_t)0xFF);
    ACCUMULATOR += MEMORY[addr];
    uint8_t sign_a_fin = ACCUMULATOR >> 7;
    SET_Z(ACCUMULATOR == 0);
    if (sign_a == sign_m) {
        SET_V(sign_a_fin != sign_a);
    }
    SET_N(sign_a_fin & 1);
}

void AND(uint16_t addr) {
    ACCUMULATOR &= MEMORY[addr];
    SET_Z(ACCUMULATOR == 0);
    SET_N((ACCUMULATOR >> 7) & 1);
}

void ASL(uint16_t addr) {
    SET_C((MEMORY[addr] >> 7) & 1);
    MEMORY[addr] = MEMORY[addr] << 1;
    SET_Z(MEMORY[addr] == 0);
    SET_N((MEMORY[addr] >> 7) & 1);
}

void BCC(uint16_t addr) {}

void BCS(uint16_t addr) {
    // TODO: fix this
    if (GET_C()) {
        PC += addr_relative(); // TODO: should it be addr-1?
    }
}

void BEQ(uint16_t addr) {
    // TODO: fix this
    if (GET_Z()) {
        PC += addr_relative(); // TODO: see BCS
    }
}

void BIT(uint16_t addr) {
    uint8_t tmp = MEMORY[addr] & ACCUMULATOR;
    SET_Z(tmp == 0);
    SET_V((tmp >> 6) & 1);
    SET_N((tmp >> 7) & 1);
}

void BMI(uint16_t addr) {
    // TODO: fix this
    if (GET_N()) {
        PC += addr_relative(); // TODO: see BCS
    }
}

void BNE(uint16_t addr) {
    // TODO: fix this
    if (!GET_Z()) {
        PC += addr_relative(); // TODO: see BCS
    }
}

void BPL(uint16_t addr) {
    // TODO: fix this
    if (!GET_N()) {
        PC += addr_relative(); // TODO: see BCS
    }
}

void BRK(uint16_t addr) {}

void BVC(uint16_t addr) {
    // TODO: fix this
    if (!GET_V()) {
        PC += addr_relative(); // TODO: see BCS
    }
}

void BVS(uint16_t addr) {
    // TODO: fix this
    if (GET_V()) {
        PC += addr_relative(); // TODO: see BCS
    }
}

void CLC(uint16_t addr) {
    SET_C(0);
}

void CLD(uint16_t addr) {
    SET_D(0);
}

void CLI(uint16_t addr) {
    SET_I(0);
}

void CLV(uint16_t addr) {
    SET_V(0);
}

void CMP(uint16_t addr) {
    uint8_t val = MEMORY[addr];
    SET_C(ACCUMULATOR >= val);
    SET_Z(ACCUMULATOR == val);
    SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
}

void CPX(uint16_t addr) {
    uint8_t val = MEMORY[addr];
    SET_C(IND_REG_X >= val);
    SET_Z(IND_REG_X == val);
    SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
}

void CPY(uint16_t addr) {
    uint8_t val = MEMORY[addr];
    SET_C(IND_REG_Y >= val);
    SET_Z(IND_REG_Y == val);
    SET_N(IS_NEG(val)); // TODO: not sure if 'val' is what the neg test should be done on
}

void DEC(uint16_t addr) {
    MEMORY[addr] -= 1; // TODO: what should happen at 0?
    uint8_t val = MEMORY[addr];
    SET_Z(val == 0); // TODO: find out if Z should be unset too or just set on zero
    SET_N(IS_NEG(val));
}

void DEX(uint16_t addr) {
    --IND_REG_X;
    SET_Z(IND_REG_X == 0); // TODO: find out if Z should be unset too or just set on zero
    SET_N(IS_NEG(IND_REG_X));
}

void DEY(uint16_t addr) {
    --IND_REG_Y;
    SET_Z(IND_REG_Y == 0);
    SET_N(IS_NEG(IND_REG_Y));
}

void EOR(uint16_t addr) {
    uint8_t val = MEMORY[addr];
    ACCUMULATOR = ACCUMULATOR ^ val;
    SET_Z(A == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void INC(uint16_t addr) {}

void INX(uint16_t addr) {
    ++IND_REG_X;
    SET_Z(IND_REG_X == 0);
    SET_N(IS_NEG(IND_REG_X));
}

void INY(uint16_t addr) {
    ++IND_REG_Y;
    SET_Z(IND_REG_Y == 0);
    SET_N(IS_NEG(IND_REG_Y));
}

void JMP(uint16_t addr) {}

void JSR(uint16_t addr)
    STACK_POINTER -= 2;
    MEMORY[STACK_POINTER + 2] = (PC & 0xFF00) >> 8;
    MEMORY[STACK_POINTER + 1] = PC & 0xFF;
    PC = addr;
}

void LDA(uint16_t addr) {
    ACCUMULATOR = MEMORY[addr];
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void LDX(uint16_t addr) {}

void LDY(uint16_t addr) {}

void LSR(uint16_t addr) {}

void NOP(uint16_t addr) {}

void ORA(uint16_t addr) {}

void PHA(uint16_t addr) {}

void PHP(uint16_t addr) {}

void PLA(uint16_t addr) {}

void PLP(uint16_t addr) {}

void ROL(uint16_t addr) {}

void ROR(uint16_t addr) {}

void RTI(uint16_t addr) {}

void RTS(uint16_t addr) {}

void SBC(uint16_t addr) {}

void SEC(uint16_t addr) {
    SET_C(1);
}

void SED(uint16_t addr) {
    SET_D(1);
}

void SEI(uint16_t addr) {
    SET_I(1);
}

void STA(uint16_t addr) {}

void STX(uint16_t addr) {}

void STY(uint16_t addr) {}

void TAX(uint16_t addr) {
    IND_REG_X = ACCUMULATOR;
    SET_Z(IND_REG_X == 0);
    SET_N(IS_NEG(IND_REG_X));
}

void TAY(uint16_t addr) {
    IND_REG_Y = ACCUMULATOR;
    SET_Z(IND_REG_Y == 0);
    SET_N(IS_NEG(IND_REG_Y));
}

void TSX(uint16_t addr) {
    IND_REG_X = STACK_POINTER;
    SET_Z(IND_REG_X == 0);
    SET_N(IS_NEG(IND_REG_X));
}

void TXA(uint16_t addr) {
    ACCUMULATOR = IND_REG_X;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void TXS(uint16_t addr) {
    STACK_POINTER = IND_REG_X;
}

void TYA(uint16_t addr) {
    ACCUMULATOR = IND_REG_Y;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void run_instruction(uint8_t opcode) {
    uint16_t addr;
    switch (opcode) {
        case 0x69: addr = addr_immediate();        ADC(addr); break;
        case 0x65: addr = addr_zero_page();        ADC(addr); break;
        case 0x75: addr = addr_zero_page_x();      ADC(addr); break;
        case 0x6D: addr = addr_abs();              ADC(addr); break;
        case 0x7D: addr = addr_abs_x();            ADC(addr); break;
        case 0x79: addr = addr_abs_y();            ADC(addr); break;
        case 0x61: addr = addr_indexed_indirect(); ADC(addr); break;
        case 0x71: addr = addr_indirect_indexed(); ADC(addr); break;
        case 0x29: addr = addr_immediate();        AND(addr); break;
        case 0x25: addr = addr_zero_page();        AND(addr); break;
        case 0x35: addr = addr_zero_page_x();      AND(addr); break;
        case 0x2D: addr = addr_abs();              AND(addr); break;
        case 0x3D: addr = addr_abs_x();            AND(addr); break;
        case 0x39: addr = addr_abs_y();            AND(addr); break;
        case 0x21: addr = addr_indexed_indirect(); AND(addr); break;
        case 0x31: addr = addr_indirect_indexed(); AND(addr); break;
        case 0x0A: addr = addr_accumulator();      ASL(addr); break;
        case 0x06: addr = addr_zero_page();        ASL(addr); break;
        case 0x16: addr = addr_zero_page_x();      ASL(addr); break;
        case 0x0E: addr = addr_abs();              ASL(addr); break;
        case 0x1E: addr = addr_abs_x();            ASL(addr); break;
        case 0x90: addr = addr_relative();         BCC(addr); break;
        case 0x80: addr = addr_relative();         BCS(addr); break;
        case 0xF0: addr = addr_relative();         BEQ(addr); break;
        case 0x24: addr = addr_zero_page();        BIT(addr); break;
        case 0x2C: addr = addr_abs();              BIT(addr); break;
        case 0x30: addr = addr_relative();         BMI(addr); break;
        case 0xD0: addr = addr_relative();         BNE(addr); break;
        case 0x10: addr = addr_relative();         BPL(addr); break;
        case 0x00: addr = addr_implied();          BRK(addr); break;
        case 0x50: addr = addr_relative();         BVC(addr); break;
        case 0x70: addr = addr_relative();         BVS(addr); break;
        case 0x18: addr = addr_implied();          CLC(addr); break;
        case 0xD8: addr = addr_implied();          CLD(addr); break;
        case 0x58: addr = addr_implied();          CLI(addr); break;
        case 0xB8: addr = addr_implied();          CLV(addr); break;
        case 0xC9: addr = addr_immediate();        CMP(addr); break;
        case 0xC5: addr = addr_zero_page();        CMP(addr); break;
        case 0xD5: addr = addr_zero_page_x();      CMP(addr); break;
        case 0xCD: addr = addr_abs();              CMP(addr); break;
        case 0xDD: addr = addr_abs_x();            CMP(addr); break;
        case 0xD9: addr = addr_abs_y();            CMP(addr); break;
        case 0xC1: addr = addr_indexed_indirect(); CMP(addr); break;
        case 0xD1: addr = addr_indirect_indexed(); CMP(addr); break;
        case 0xE0: addr = addr_immediate();        CPX(addr); break;
        case 0xE4: addr = addr_zero_page();        CPX(addr); break;
        case 0xEC: addr = addr_abs();              CPX(addr); break;
        case 0xC0: addr = addr_immediate();        CPY(addr); break;
        case 0xC4: addr = addr_zero_page();        CPY(addr); break;
        case 0xCC: addr = addr_abs();              CPY(addr); break;
        case 0xC6: addr = addr_zero_page();        DEC(addr); break;
        case 0xD6: addr = addr_zero_page_x();      DEC(addr); break;
        case 0xCE: addr = addr_abs();              DEC(addr); break;
        case 0xDE: addr = addr_abs_x();            DEC(addr); break;
        case 0xCA: addr = addr_implied();          DEX(addr); break;
        case 0x88: addr = addr_implied();          DEY(addr); break;
        case 0x49: addr = addr_immediate();        EOR(addr); break;
        case 0x45: addr = addr_zero_page();        EOR(addr); break;
        case 0x55: addr = addr_zero_page_x();      EOR(addr); break;
        case 0x4D: addr = addr_abs();              EOR(addr); break;
        case 0x5D: addr = addr_abs_x();            EOR(addr); break;
        case 0x59: addr = addr_abs_y();            EOR(addr); break;
        case 0x41: addr = addr_indexed_indirect(); EOR(addr); break;
        case 0x51: addr = addr_indirect_indexed(); EOR(addr); break;
        case 0xE6: addr = addr_zero_page();        INC(addr); break;
        case 0xF6: addr = addr_zero_page_x();      INC(addr); break;
        case 0xEE: addr = addr_abs();              INC(addr); break;
        case 0xFE: addr = addr_abs_x();            INC(addr); break;
        case 0xE8: addr = addr_implied();          INX(addr); break;
        case 0xC8: addr = addr_implied();          INY(addr); break;
        case 0x4C: addr = addr_abs();              JMP(addr); break;
        case 0x6C: addr = 0; /* indirect */        JMP(addr); break;
        case 0x20: addr = addr_abs();              JSR(addr); break;
        case 0xA9: addr = addr_immediate();        LDA(addr); break;
        case 0xA5: addr = addr_zero_page();        LDA(addr); break;
        case 0xB5: addr = addr_zero_page_x();      LDA(addr); break;
        case 0xAD: addr = addr_abs();              LDA(addr); break;
        case 0xBD: addr = addr_abs_x();            LDA(addr); break;
        case 0xB9: addr = addr_abs_y();            LDA(addr); break;
        case 0xA1: addr = addr_indexed_indirect(); LDA(addr); break;
        case 0xB1: addr = addr_indirect_indexed(); LDA(addr); break;
        case 0xA2: addr = addr_immediate();        LDX(addr); break;
        case 0xA6: addr = addr_zero_page();        LDX(addr); break;
        case 0xB6: addr = addr_zero_page_y();      LDX(addr); break;
        case 0xAE: addr = addr_abs();              LDX(addr); break;
        case 0xBE: addr = addr_abs_y();            LDX(addr); break;
        case 0xA0: addr = addr_immediate();        LDY(addr); break;
        case 0xA4: addr = addr_zero_page();        LDY(addr); break;
        case 0xB4: addr = addr_zero_page_x();      LDY(addr); break;
        case 0xAC: addr = addr_abs();              LDY(addr); break;
        case 0xBC: addr = addr_abs_x();            LDY(addr); break;
        case 0x4A: addr = addr_accumulator();      LSR(addr); break;
        case 0x46: addr = addr_zero_page();        LSR(addr); break;
        case 0x56: addr = addr_zero_page_x();      LSR(addr); break;
        case 0x4E: addr = addr_abs();              LSR(addr); break;
        case 0x5E: addr = addr_abs_x();            LSR(addr); break;
        case 0xEA: addr = addr_implied();          NOP(addr); break;
        case 0x09: addr = addr_immediate();        ORA(addr); break;
        case 0x05: addr = addr_zero_page();        ORA(addr); break;
        case 0x15: addr = addr_zero_page_x();      ORA(addr); break;
        case 0x0D: addr = addr_abs();              ORA(addr); break;
        case 0x1D: addr = addr_abs_x();            ORA(addr); break;
        case 0x19: addr = addr_abs_y();            ORA(addr); break;
        case 0x01: addr = addr_indexed_indirect(); ORA(addr); break;
        case 0x11: addr = addr_indirect_indexed(); ORA(addr); break;
        case 0x48: addr = addr_implied();          PHA(addr); break;
        case 0x08: addr = addr_implied();          PHP(addr); break;
        case 0x68: addr = addr_implied();          PLA(addr); break;
        case 0x28: addr = addr_implied();          PLP(addr); break;
        case 0x2A: addr = addr_accumulator();      ROL(addr); break;
        case 0x26: addr = addr_zero_page();        ROL(addr); break;
        case 0x36: addr = addr_zero_page_x();      ROL(addr); break;
        case 0x2E: addr = addr_abs();              ROL(addr); break;
        case 0x3E: addr = addr_abs_x();            ROL(addr); break;
        case 0x6A: addr = addr_accumulator();      ROR(addr); break;
        case 0x66: addr = addr_zero_page();        ROR(addr); break;
        case 0x76: addr = addr_zero_page_x();      ROR(addr); break;
        case 0x6E: addr = addr_abs();              ROR(addr); break;
        case 0x7E: addr = addr_abs_x();            ROR(addr); break;
        case 0x40: addr = addr_implied();          RTI(addr); break;
        case 0x60: addr = addr_implied();          RTS(addr); break;
        case 0xE9: addr = addr_immediate();        SBC(addr); break;
        case 0xE5: addr = addr_zero_page();        SBC(addr); break;
        case 0xF5: addr = addr_zero_page_x();      SBC(addr); break;
        case 0xED: addr = addr_abs();              SBC(addr); break;
        case 0xFD: addr = addr_abs_x();            SBC(addr); break;
        case 0xF9: addr = addr_abs_y();            SBC(addr); break;
        case 0xE1: addr = addr_indexed_indirect(); SBC(addr); break;
        case 0xF1: addr = addr_indirect_indexed(); SBC(addr); break;
        case 0x38: addr = addr_implied();          SEC(addr); break;
        case 0xF8: addr = addr_implied();          SED(addr); break;
        case 0x78: addr = addr_implied();          SEI(addr); break;
        case 0x85: addr = addr_zero_page();        STA(addr); break;
        case 0x95: addr = addr_zero_page_x();      STA(addr); break;
        case 0x8D: addr = addr_abs();              STA(addr); break;
        case 0x9D: addr = addr_abs_x();            STA(addr); break;
        case 0x99: addr = addr_abs_y();            STA(addr); break;
        case 0x81: addr = addr_indexed_indirect(); STA(addr); break;
        case 0x91: addr = addr_indirect_indexed(); STA(addr); break;
        case 0x86: addr = addr_zero_page();        STA(addr); break;
        case 0x96: addr = addr_zero_page_y();      STA(addr); break;
        case 0x8E: addr = addr_abs();              STA(addr); break;
        case 0x84: addr = addr_zero_page();        STY(addr); break;
        case 0x94: addr = addr_zero_page_x();      STY(addr); break;
        case 0x8C: addr = addr_abs();              STY(addr); break;
        case 0xAA: addr = addr_implied();          TAX(addr); break;
        case 0xA8: addr = addr_implied();          TAY(addr); break;
        case 0xBA: addr = addr_implied();          TSX(addr); break;
        case 0x8A: addr = addr_implied();          TXA(addr); break;
        case 0x9A: addr = addr_implied();          TXS(addr); break;
        case 0x98: addr = addr_implied();          TYA(addr); break;
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
