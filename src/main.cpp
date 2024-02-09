#include <cstdio>
#include <cstdint>

#define IS_NEG(uint8_num) (1 == uint8_num >> 7)
#define SET_C(b) ((STATUS_REG &= 0b11111110) | (uint8_t)(b))
#define SET_Z(b) ((STATUS_REG &= 0b11111101) | ((uint8_t)(b) << 1))
#define SET_I(b) ((STATUS_REG &= 0b11111011) | ((uint8_t)(b) << 2))
#define SET_D(b) ((STATUS_REG &= 0b11110111) | ((uint8_t)(b) << 3))
#define SET_B(b) ((STATUS_REG &= 0b11110111) | ((uint8_t)(b) << 4))
#define SET_V(b) ((STATUS_REG &= 0b10111111) | ((uint8_t)(b) << 6))
#define SET_N(b) ((STATUS_REG &= 0b01111111) | ((uint8_t)(b) << 7))
#define GET_C() ((STATUS_REG & 0b00000001) > 0)
#define GET_Z() ((STATUS_REG & 0b00000010) > 0)
#define GET_I() ((STATUS_REG & 0b00000100) > 0)
#define GET_D() ((STATUS_REG & 0b00001000) > 0)
#define GET_B() ((STATUS_REG & 0b00010000) > 0)
#define GET_V() ((STATUS_REG & 0b01000000) > 0)
#define GET_N() ((STATUS_REG & 0b10000000) > 0)

uint16_t PC;
// TODO: handle the stack pointer offset, maybe make some stack op funcs/macros
uint8_t STACK_POINTER; //stack is 0x0100 - 0x01FF in memory, pointer is least significant half
uint8_t ACCUMULATOR;
uint8_t IND_REG_X;
uint8_t IND_REG_Y;
uint8_t STATUS_REG;
// Note: supposedly the NES CPU lacks a decimal mode, so the D status flag
// shouldn't affect anything, need to make sure
// Note: bit 5 may always be set, according to nesdev.org/6502_cpu.txt

uint8_t MEMORY[0xFFFF];
// mapped to 0x0000 - 0x07FF in hardware
// 0x0800 - 0x0FFF, 0x1000 - 0x17FF, 0x1800 - 0x1FFF
// are mirrors of RAM addresses
// mapped to 0x2000 - 0x2007 in hardware
// 0x2008 - 0x3FFF are mirrors of PPU addresses, repeats every 8 bytes
// mapped to 0x4000 - 0x4017 in hardware
// mapped to 0x4020 - 0xFFFF


// -- Addressing Mode Helpers --
uint16_t addr_zero_page() {
    return (uint16_t)MEMORY[++PC];
}

uint16_t addr_zero_page_x() {
    uint16_t addr = (uint16_t)MEMORY[++PC];
    return (addr + IND_REG_X) & 0x00FFu; // zero page wrap around
}

uint16_t addr_zero_page_y() {
    uint16_t addr = (uint16_t)MEMORY[++PC];
    return (addr + IND_REG_Y) & 0x00FFu; // zero page wrap around
}

uint16_t addr_abs() {
    PC += 2;
    return (uint16_t)(((uint16_t)MEMORY[PC] << 8) | (uint16_t)MEMORY[PC - 1]);
}

uint16_t addr_abs_x() {
    PC += 2;
    uint16_t addr = (uint16_t)(((uint16_t)MEMORY[PC] << 8) | (uint16_t)MEMORY[PC - 1]);
    return (uint16_t)(addr + IND_REG_X); // allow 16 bit wrap around
}

uint16_t addr_abs_y() {
    PC += 2;
    uint16_t addr = (uint16_t)(((uint16_t)MEMORY[PC] << 8) | (uint16_t)MEMORY[PC - 1]);
    return (uint16_t)(addr + IND_REG_Y); // allow 16 bit wrap around
}

uint16_t addr_indexed_indirect() {
    uint8_t pt = (uint8_t)(MEMORY[++PC] + IND_REG_X); // allow zero page wrap around
    // note: full addr should always be in zero page, meaning pt + 1 should NOT
    //       cross the zero page boundary
    // TODO: figure out what to do if it does cross the boundary
    return (uint16_t)(((uint16_t)MEMORY[pt + 1] << 8) | (uint16_t)MEMORY[pt]);
}

uint16_t addr_indirect_indexed() {
    uint16_t pt = (uint16_t)MEMORY[MEMORY[++PC]];
    return (uint16_t)((((uint16_t)MEMORY[pt + 1] << 8) | (uint16_t)MEMORY[pt]) + IND_REG_Y);
}

// TODO: apparently some games (rare) use unofficial opcodes, may need to
//       implement them
// -- Opcodes --
// Load/Store Operations
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

void STA(uint8_t* pt) {
    *pt = ACCUMULATOR;
}

void STX(uint8_t* pt) {
    *pt = IND_REG_X;
}

void STY(uint8_t* pt) {
    *pt = IND_REG_Y;
}

// Register Transfers
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

void TXA() {
    ACCUMULATOR = IND_REG_X;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void TYA() {
    ACCUMULATOR = IND_REG_Y;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

// Stack Operation
void TSX() {
    IND_REG_X = STACK_POINTER;
    SET_Z(IND_REG_X == 0);
    SET_N(IS_NEG(IND_REG_X));
}


void TXS() {
    STACK_POINTER = IND_REG_X;
}

void PHA() {
    MEMORY[STACK_POINTER] = ACCUMULATOR;
    --STACK_POINTER;
}

void PHP() {
    MEMORY[STACK_POINTER] = STATUS_REG;
    --STACK_POINTER;
}

void PLA() {
    ++STACK_POINTER;
    ACCUMULATOR = MEMORY[STACK_POINTER];
}

void PLP() {
    ++STACK_POINTER;
    STATUS_REG = MEMORY[STACK_POINTER];
}

// Logical
void AND(uint8_t* pt) {
    ACCUMULATOR &= *pt;
    SET_Z(ACCUMULATOR == 0);
    SET_N((ACCUMULATOR >> 7) & 1);
}

void EOR(uint8_t* pt) {
    uint8_t val = *pt;
    ACCUMULATOR = (uint8_t)(ACCUMULATOR ^ val);
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void ORA(uint8_t* pt) {
    ACCUMULATOR |= *pt;
    SET_Z(ACCUMULATOR == 0);
    SET_N(IS_NEG(ACCUMULATOR));
}

void BIT(uint8_t* pt) {
    uint8_t tmp = (uint8_t)(*pt & ACCUMULATOR);
    SET_Z(tmp == 0);
    SET_V((tmp >> 6) & 1);
    SET_N((tmp >> 7) & 1);
}

// Arithmetic
void ADC(uint8_t* pt) {
    uint16_t tmp = (uint16_t)((uint16_t)ACCUMULATOR + (uint16_t)*pt + (uint16_t)GET_C());
    SET_C(tmp > 255);
    SET_V(tmp > 255);
    SET_Z(tmp == 0);
    ACCUMULATOR = (uint8_t)(tmp & 0x00FFu);
    SET_N((ACCUMULATOR >> 7) & 1);
}

void SBC(uint8_t* pt) {
    int16_t tmp = (int16_t)((int16_t)ACCUMULATOR - (int16_t)*pt - (int16_t)!GET_C());
    SET_C(tmp >= 0);
    SET_V(tmp > 127 || tmp < -127);
    SET_Z(tmp == 0);
    ACCUMULATOR = (uint8_t)(tmp & 0x00FFu);
    SET_N((ACCUMULATOR >> 7) & 1);
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

// Increments & Decrements
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

void INC(uint8_t* pt) {
    ++(*pt);
    SET_Z(*pt == 0);
    SET_N(IS_NEG(*pt));
}

void DEC(uint8_t* pt) {
    *pt -= 1; // TODO: what should happen at 0?
    uint8_t val = *pt;
    SET_Z(val == 0); // TODO: find out if Z should be unset too or just set on zero
    SET_N(IS_NEG(val));
}

// Shifts
void ASL(uint8_t* pt) {
    SET_C((*pt >> 7) & 1);
    *pt = (uint8_t)(*pt << 1);
    SET_Z(*pt == 0);
    SET_N((*pt >> 7) & 1);
}

void LSR(uint8_t* pt) {
    SET_C(*pt & 0x01);
    *pt = (uint8_t)(*pt >> 1);
    SET_Z(*pt == 0);
    SET_N(false); // high bit is always 0 after shift
}

void ROL(uint8_t* pt) {
    bool old_c = GET_C();
    SET_C((*pt >> 7) & 1);
    *pt = (uint8_t)(*pt << 1);
    *pt |= (uint8_t)old_c;
    SET_Z(*pt == 0);
    SET_N((*pt >> 7) & 1);
}

void ROR(uint8_t* pt) {
    bool old_c = GET_C();
    SET_C(*pt & 1);
    *pt = (uint8_t)(*pt >> 1);
    *pt |= (((uint8_t)old_c) << 7);
    SET_Z(*pt == 0);
    SET_N((*pt >> 7) & 1);
}

// Jumps & Calls
void JMP(uint16_t addr, bool indirect) {
    if (indirect) {
        PC = (uint16_t)((((uint16_t)MEMORY[addr+1]) << 8) | ((uint16_t)MEMORY[addr]));
        return;
    }
    PC = addr;
}

void JSR(uint16_t addr) {
    STACK_POINTER -= 2;
    MEMORY[STACK_POINTER + 2] = (uint8_t)((PC & 0xFF00) >> 8);
    MEMORY[STACK_POINTER + 1] = (uint8_t)(PC & 0xFF);
    PC = addr;
}

void RTS() {
    STACK_POINTER += 2;
    PC = (uint16_t)(((uint16_t)MEMORY[STACK_POINTER] << 8) | (uint16_t)MEMORY[STACK_POINTER - 1]);
}

// Branches
void BCC(uint8_t* pt) {
    if (!GET_C()) PC += (int8_t)(*pt);
}

void BCS(uint8_t* pt) {
    if (GET_C()) PC += (int8_t)(*pt);
}

void BEQ(uint8_t* pt) {
    if (GET_Z()) PC += (int8_t)(*pt);
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

void BVC(uint8_t* pt) {
    if (!GET_V()) PC += (int8_t)(*pt);
}

void BVS(uint8_t* pt) {
    if (GET_V()) PC += (int8_t)(*pt);
}

// Status Flag Changes
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

void SEC() {
    SET_C(1);
}

void SED() {
    SET_D(1);
}

void SEI() {
    SET_I(1);
}

// System Functions
void BRK() {
    STACK_POINTER -= 3;
    SET_B(1); // Note: does this get set then pushed onto the stack? or after
    MEMORY[STACK_POINTER + 3] = (uint8_t)((PC & 0xFF00) >> 8);
    MEMORY[STACK_POINTER + 2] = (uint8_t)(PC & 0xFF);
    MEMORY[STACK_POINTER + 1] = STATUS_REG;
    PC = (uint16_t)((((uint16_t)MEMORY[0xFFFF]) << 8) | ((uint16_t)MEMORY[0xFFFE]));
}

void NOP() {} // Note: probably don't need this

void RTI() {
    STACK_POINTER += 3;
    STATUS_REG = MEMORY[STACK_POINTER - 2];
    PC = (uint16_t)(((uint16_t)MEMORY[STACK_POINTER] << 8) | (uint16_t)MEMORY[STACK_POINTER - 1]);
}


void run_instruction(uint8_t opcode) {
    uint16_t addr;
    switch (opcode) {
        // Load/Store Operations
        case 0xA9: addr = ++PC;                    LDA(&MEMORY[addr]); break;
        case 0xA5: addr = addr_zero_page();        LDA(&MEMORY[addr]); break;
        case 0xB5: addr = addr_zero_page_x();      LDA(&MEMORY[addr]); break;
        case 0xAD: addr = addr_abs();              LDA(&MEMORY[addr]); break;
        case 0xBD: addr = addr_abs_x();            LDA(&MEMORY[addr]); break;
        case 0xB9: addr = addr_abs_y();            LDA(&MEMORY[addr]); break;
        case 0xA1: addr = addr_indexed_indirect(); LDA(&MEMORY[addr]); break;
        case 0xB1: addr = addr_indirect_indexed(); LDA(&MEMORY[addr]); break;
        case 0xA2: addr = ++PC;                    LDX(&MEMORY[addr]); break;
        case 0xA6: addr = addr_zero_page();        LDX(&MEMORY[addr]); break;
        case 0xB6: addr = addr_zero_page_y();      LDX(&MEMORY[addr]); break;
        case 0xAE: addr = addr_abs();              LDX(&MEMORY[addr]); break;
        case 0xBE: addr = addr_abs_y();            LDX(&MEMORY[addr]); break;
        case 0xA0: addr = ++PC;                    LDY(&MEMORY[addr]); break;
        case 0xA4: addr = addr_zero_page();        LDY(&MEMORY[addr]); break;
        case 0xB4: addr = addr_zero_page_x();      LDY(&MEMORY[addr]); break;
        case 0xAC: addr = addr_abs();              LDY(&MEMORY[addr]); break;
        case 0xBC: addr = addr_abs_x();            LDY(&MEMORY[addr]); break;
        case 0x85: addr = addr_zero_page();        STA(&MEMORY[addr]); break;
        case 0x95: addr = addr_zero_page_x();      STA(&MEMORY[addr]); break;
        case 0x8D: addr = addr_abs();              STA(&MEMORY[addr]); break;
        case 0x9D: addr = addr_abs_x();            STA(&MEMORY[addr]); break;
        case 0x99: addr = addr_abs_y();            STA(&MEMORY[addr]); break;
        case 0x81: addr = addr_indexed_indirect(); STA(&MEMORY[addr]); break;
        case 0x91: addr = addr_indirect_indexed(); STA(&MEMORY[addr]); break;
        case 0x86: addr = addr_zero_page();        STX(&MEMORY[addr]); break;
        case 0x96: addr = addr_zero_page_y();      STX(&MEMORY[addr]); break;
        case 0x8E: addr = addr_abs();              STX(&MEMORY[addr]); break;
        case 0x84: addr = addr_zero_page();        STY(&MEMORY[addr]); break;
        case 0x94: addr = addr_zero_page_x();      STY(&MEMORY[addr]); break;
        case 0x8C: addr = addr_abs();              STY(&MEMORY[addr]); break;

        // Register Transfers
        case 0xAA: /* implied */                   TAX(); break;
        case 0xA8: /* implied */                   TAY(); break;
        case 0x8A: /* implied */                   TXA(); break;
        case 0x98: /* implied */                   TYA(); break;

        // Stack Operation
        case 0xBA: /* implied */                   TSX(); break;
        case 0x9A: /* implied */                   TXS(); break;
        case 0x48: /* implied */                   PHA(); break;
        case 0x08: /* implied */                   PHP(); break;
        case 0x68: /* implied */                   PLA(); break;
        case 0x28: /* implied */                   PLP(); break;

        // Logical
        case 0x29: addr = ++PC;                    AND(&MEMORY[addr]); break;
        case 0x25: addr = addr_zero_page();        AND(&MEMORY[addr]); break;
        case 0x35: addr = addr_zero_page_x();      AND(&MEMORY[addr]); break;
        case 0x2D: addr = addr_abs();              AND(&MEMORY[addr]); break;
        case 0x3D: addr = addr_abs_x();            AND(&MEMORY[addr]); break;
        case 0x39: addr = addr_abs_y();            AND(&MEMORY[addr]); break;
        case 0x21: addr = addr_indexed_indirect(); AND(&MEMORY[addr]); break;
        case 0x31: addr = addr_indirect_indexed(); AND(&MEMORY[addr]); break;
        case 0x49: addr = ++PC;                    EOR(&MEMORY[addr]); break;
        case 0x45: addr = addr_zero_page();        EOR(&MEMORY[addr]); break;
        case 0x55: addr = addr_zero_page_x();      EOR(&MEMORY[addr]); break;
        case 0x4D: addr = addr_abs();              EOR(&MEMORY[addr]); break;
        case 0x5D: addr = addr_abs_x();            EOR(&MEMORY[addr]); break;
        case 0x59: addr = addr_abs_y();            EOR(&MEMORY[addr]); break;
        case 0x41: addr = addr_indexed_indirect(); EOR(&MEMORY[addr]); break;
        case 0x51: addr = addr_indirect_indexed(); EOR(&MEMORY[addr]); break;
        case 0x09: addr = ++PC;                    ORA(&MEMORY[addr]); break;
        case 0x05: addr = addr_zero_page();        ORA(&MEMORY[addr]); break;
        case 0x15: addr = addr_zero_page_x();      ORA(&MEMORY[addr]); break;
        case 0x0D: addr = addr_abs();              ORA(&MEMORY[addr]); break;
        case 0x1D: addr = addr_abs_x();            ORA(&MEMORY[addr]); break;
        case 0x19: addr = addr_abs_y();            ORA(&MEMORY[addr]); break;
        case 0x01: addr = addr_indexed_indirect(); ORA(&MEMORY[addr]); break;
        case 0x11: addr = addr_indirect_indexed(); ORA(&MEMORY[addr]); break;
        case 0x24: addr = addr_zero_page();        BIT(&MEMORY[addr]); break;
        case 0x2C: addr = addr_abs();              BIT(&MEMORY[addr]); break;

        // Arithmetic
        case 0x69: addr = ++PC;                    ADC(&MEMORY[addr]); break;
        case 0x65: addr = addr_zero_page();        ADC(&MEMORY[addr]); break;
        case 0x75: addr = addr_zero_page_x();      ADC(&MEMORY[addr]); break;
        case 0x6D: addr = addr_abs();              ADC(&MEMORY[addr]); break;
        case 0x7D: addr = addr_abs_x();            ADC(&MEMORY[addr]); break;
        case 0x79: addr = addr_abs_y();            ADC(&MEMORY[addr]); break;
        case 0x61: addr = addr_indexed_indirect(); ADC(&MEMORY[addr]); break;
        case 0x71: addr = addr_indirect_indexed(); ADC(&MEMORY[addr]); break;
        case 0xE9: addr = ++PC;                    SBC(&MEMORY[addr]); break;
        case 0xE5: addr = addr_zero_page();        SBC(&MEMORY[addr]); break;
        case 0xF5: addr = addr_zero_page_x();      SBC(&MEMORY[addr]); break;
        case 0xED: addr = addr_abs();              SBC(&MEMORY[addr]); break;
        case 0xFD: addr = addr_abs_x();            SBC(&MEMORY[addr]); break;
        case 0xF9: addr = addr_abs_y();            SBC(&MEMORY[addr]); break;
        case 0xE1: addr = addr_indexed_indirect(); SBC(&MEMORY[addr]); break;
        case 0xF1: addr = addr_indirect_indexed(); SBC(&MEMORY[addr]); break;
        case 0xC9: addr = ++PC;                    CMP(&MEMORY[addr]); break;
        case 0xC5: addr = addr_zero_page();        CMP(&MEMORY[addr]); break;
        case 0xD5: addr = addr_zero_page_x();      CMP(&MEMORY[addr]); break;
        case 0xCD: addr = addr_abs();              CMP(&MEMORY[addr]); break;
        case 0xDD: addr = addr_abs_x();            CMP(&MEMORY[addr]); break;
        case 0xD9: addr = addr_abs_y();            CMP(&MEMORY[addr]); break;
        case 0xC1: addr = addr_indexed_indirect(); CMP(&MEMORY[addr]); break;
        case 0xD1: addr = addr_indirect_indexed(); CMP(&MEMORY[addr]); break;
        case 0xE0: addr = ++PC;                    CPX(&MEMORY[addr]); break;
        case 0xE4: addr = addr_zero_page();        CPX(&MEMORY[addr]); break;
        case 0xEC: addr = addr_abs();              CPX(&MEMORY[addr]); break;
        case 0xC0: addr = ++PC;                    CPY(&MEMORY[addr]); break;
        case 0xC4: addr = addr_zero_page();        CPY(&MEMORY[addr]); break;
        case 0xCC: addr = addr_abs();              CPY(&MEMORY[addr]); break;

        // Increments & Decrements
        case 0xE8: /* implied */                   INX(); break;
        case 0xC8: /* implied */                   INY(); break;
        case 0xCA: /* implied */                   DEX(); break;
        case 0x88: /* implied */                   DEY(); break;
        case 0xE6: addr = addr_zero_page();        INC(&MEMORY[addr]); break;
        case 0xF6: addr = addr_zero_page_x();      INC(&MEMORY[addr]); break;
        case 0xEE: addr = addr_abs();              INC(&MEMORY[addr]); break;
        case 0xFE: addr = addr_abs_x();            INC(&MEMORY[addr]); break;
        case 0xC6: addr = addr_zero_page();        DEC(&MEMORY[addr]); break;
        case 0xD6: addr = addr_zero_page_x();      DEC(&MEMORY[addr]); break;
        case 0xCE: addr = addr_abs();              DEC(&MEMORY[addr]); break;
        case 0xDE: addr = addr_abs_x();            DEC(&MEMORY[addr]); break;

        // Shifts
        case 0x0A:                                 ASL(&ACCUMULATOR); break;
        case 0x06: addr = addr_zero_page();        ASL(&MEMORY[addr]); break;
        case 0x16: addr = addr_zero_page_x();      ASL(&MEMORY[addr]); break;
        case 0x0E: addr = addr_abs();              ASL(&MEMORY[addr]); break;
        case 0x1E: addr = addr_abs_x();            ASL(&MEMORY[addr]); break;
        case 0x4A:                                 LSR(&ACCUMULATOR); break;
        case 0x46: addr = addr_zero_page();        LSR(&MEMORY[addr]); break;
        case 0x56: addr = addr_zero_page_x();      LSR(&MEMORY[addr]); break;
        case 0x4E: addr = addr_abs();              LSR(&MEMORY[addr]); break;
        case 0x5E: addr = addr_abs_x();            LSR(&MEMORY[addr]); break;
        case 0x2A:                                 ROL(&ACCUMULATOR); break;
        case 0x26: addr = addr_zero_page();        ROL(&MEMORY[addr]); break;
        case 0x36: addr = addr_zero_page_x();      ROL(&MEMORY[addr]); break;
        case 0x2E: addr = addr_abs();              ROL(&MEMORY[addr]); break;
        case 0x3E: addr = addr_abs_x();            ROL(&MEMORY[addr]); break;
        case 0x6A:                                 ROR(&ACCUMULATOR); break;
        case 0x66: addr = addr_zero_page();        ROR(&MEMORY[addr]); break;
        case 0x76: addr = addr_zero_page_x();      ROR(&MEMORY[addr]); break;
        case 0x6E: addr = addr_abs();              ROR(&MEMORY[addr]); break;
        case 0x7E: addr = addr_abs_x();            ROR(&MEMORY[addr]); break;

        // Jumps & Calls
        case 0x4C: addr = addr_abs();              JMP(addr, false); break; // jmp absolute
        case 0x6C: addr = addr_abs();              JMP(addr, true); break; // jmp indirect
        case 0x20: addr = addr_abs();              JSR(addr); break; // this might be different like jmp
        case 0x60: /* implied */                   RTS();   break;

        // Branches
        case 0x90: addr = ++PC;                    BCC(&MEMORY[addr]); break;
        case 0x80: addr = ++PC;                    BCS(&MEMORY[addr]); break;
        case 0xF0: addr = ++PC;                    BEQ(&MEMORY[addr]); break;
        case 0x30: addr = ++PC;                    BMI(&MEMORY[addr]); break;
        case 0xD0: addr = ++PC;                    BNE(&MEMORY[addr]); break;
        case 0x10: addr = ++PC;                    BPL(&MEMORY[addr]); break;
        case 0x50: addr = ++PC;                    BVC(&MEMORY[addr]); break;
        case 0x70: addr = ++PC;                    BVS(&MEMORY[addr]); break;

        // Status Flag Changes
        case 0x18: /* implied */                   CLC();  break;
        case 0xD8: /* implied */                   CLD();  break;
        case 0x58: /* implied */                   CLI();  break;
        case 0xB8: /* implied */                   CLV();  break;
        case 0x38: /* implied */                   SEC();  break;
        case 0xF8: /* implied */                   SED();  break;
        case 0x78: /* implied */                   SEI();  break;

        // System Functions
        case 0x00: /* implied */                   BRK();  break;
        case 0xEA: /* implied */                   NOP();  break;
        case 0x40: /* implied */                   RTI();  break;

        default:
            // note: illegal or undocumented opcode
            // decide on whether or not we want to implement the undocumented ones
            // not sure if any ROMS use those
            break;
    }
}

// -- Testing Helpers --
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
    std::printf("nes emulator\n");

    // DEX
    reset();
    IND_REG_X = 2;
    run_instruction(0xCA);
    if (IND_REG_X != 1 && !GET_Z() && !GET_N()) std::printf("DEX\n");
    run_instruction(0xCA);
    if (IND_REG_X != 0 && GET_Z() && !GET_N()) std::printf("DEX zero\n");
    run_instruction(0xCA);
    if (IND_REG_X != 0xFF && !GET_Z() && GET_N()) std::printf("DEX neg\n");

    // test to see if we can get index from pt and memory
    uint8_t* pt = &(MEMORY[576]);
    std::printf("MEMORY[%u]\n", (uint16_t)(pt - MEMORY));
}
