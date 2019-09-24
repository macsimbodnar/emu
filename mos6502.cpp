#include "log.hpp"
#include "mos6502.hpp"


MOS6502::MOS6502(Bus *b) :
    A(0x00), X(0x00), Y(0x00), PC(0x0000), S(0xFD), P(0x00), bus(b),
    cycles(0), accumulator_addressing(false) {

    if (bus == nullptr) {
        log_e("Bus is nullptr");
    }
}


void MOS6502::set_flag(const status_flag_t flag, const bool val) {
    if (val) {
        S |= flag;
    } else {
        S &= ~flag;
    }
}


bool MOS6502::read_flag(const status_flag_t flag) {
    return (S & flag);
}


void MOS6502::mem_fetch() {
    if (accumulator_addressing) {
        data_bus = A;
        accumulator_addressing = false;
    } else {
        bus->access(address, Bus::READ, data_bus);
    }
}

void MOS6502::mem_write() {
    bus->access(address, Bus::WRITE, data_bus);
}


void MOS6502::clock() {
    if (cycles == 0) {

        address = PC++;
        mem_fetch();
        opcode = data_bus;

        cycles = opcode_table[opcode].cycles;

        bool add = (this->*opcode_table[opcode].addrmode)();
        bool opp = (this->*opcode_table[opcode].operation)();

        if (opp & add) {
            cycles += 2;
        } else if (opp || add) {
            cycles++;
        }

        // Always set the unused status flag bit to 1
        set_flag(U, true);
    }

    // cycles--;
    cycles = 0;
}


void MOS6502::reset() {
    // Reset registers
    A = 0x00;
    X = 0x00;
    Y = 0x00;

    S = 0xFD;
    P = 0x00 | U;

    // Read from fix mem address to jump to programmable location
    address = 0xFFFC;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address++;
    mem_fetch();
    PC = (((uint16_t)data_bus) << 8) | tmp_buff;

    // Clear helpers
    relative_adderess = 0x0000;
    address = 0x0000;
    data_bus = 0x00;
    accumulator_addressing = false;

    cycles = 8;
}


void MOS6502::irq() {   // Read from 0xFFFE
    if (read_flag(I) == false) {
        // Push PC on the stack
        // Write first the high because the stack decrease
        address = 0x0100 + S--;
        data_bus = (PC >> 8) & 0x00FF;
        mem_write();    // write high byte

        address = 0x0100 + S--;
        data_bus = PC & 0x00FF;
        mem_write();    // write low byte

        // Push status on stack
        set_flag(B, 0);
        set_flag(U, 1);
        set_flag(I, 1);

        data_bus = P;
        address = 0x0100 + S--;
        mem_write();

        // Read new PC from the fixed location
        address = 0xFFFE;
        mem_fetch();
        tmp_buff = data_bus & 0x00FF;
        address++;
        mem_fetch();
        PC = (((uint16_t)data_bus) << 8) | tmp_buff;

        cycles = 7;
    }
}


void MOS6502::nmi() {   // Read from 0xFFFA
    // Push PC on the stack
    // Write first the high because the stack decrease
    address = 0x0100 + S--;
    data_bus = (PC >> 8) & 0x00FF;
    mem_write();    // write high byte

    address = 0x0100 + S--;
    data_bus = PC & 0x00FF;
    mem_write();    // write low byte

    // Push status on stack
    set_flag(B, 0);
    set_flag(U, 1);
    set_flag(I, 1);

    data_bus = P;
    address = 0x0100 + S--;
    mem_write();

    // Read new PC from the fixed location
    address = 0xFFFA;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address++;
    mem_fetch();
    PC = (((uint16_t)data_bus) << 8) | tmp_buff;

    cycles = 8;
}


p_state_t MOS6502::get_status() {
    p_state_t state;

    state.A = A;
    state.X = X;
    state.Y = Y;
    state.S = S;
    state.PC = PC;

    state.N = read_flag(N);
    state.O = read_flag(O);
    state.U = read_flag(U);
    state.B = read_flag(B);
    state.D = read_flag(D);
    state.I = read_flag(I);
    state.Z = read_flag(Z);
    state.C = read_flag(C);

    state.opcode_name = opcode_table[opcode].name;
    state.opcode = opcode;

    state.data_bus = data_bus;
    state.address = address;
    state.relative_adderess = relative_adderess;

    state.tmp_buff = tmp_buff;

    state.cycles_count = cycles;
    state.cycles_needed = opcode_table[opcode].cycles;

    return state;
}


/********************************************************
 *                  ADDRESSING MODES                    *
 ********************************************************/
bool MOS6502::ACC() {   // DONE
    accumulator_addressing = true;
    return false;
}

bool MOS6502::IMM() {   // DONE
    address = PC++;
    return false;
}

bool MOS6502::ABS() {   // DONE
    address = PC++;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address = PC++;
    mem_fetch();
    address = (((uint16_t)data_bus) << 8) | tmp_buff;
    return false;
}

bool MOS6502::ZPI() {   // DONE
    address = PC++;
    mem_fetch();
    address = data_bus & 0x00FF;
    return false;
}

bool MOS6502::ZPX() {   // DONE
    address = PC++;
    mem_fetch();
    address = (data_bus + X) & 0x00FF;
    return false;
}

bool MOS6502::ZPY() {   // DONE
    address = PC++;
    mem_fetch();
    address = (data_bus + Y) & 0x00FF;
    return false;
}

bool MOS6502::ABX() {   // DONE
    address = PC++;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address = PC++;
    mem_fetch();
    address = ((((uint16_t)data_bus) << 8) | tmp_buff) + X;

    if ((address & 0xFF00) != (((uint16_t)data_bus) << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::ABY() {   // DONE
    address = PC++;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address = PC++;
    mem_fetch();
    address = ((((uint16_t)data_bus) << 8) | tmp_buff) + Y;

    if ((address & 0xFF00) != (((uint16_t)data_bus) << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::IMP() {   // DONE
    return false;
}

bool MOS6502::REL() {   // DONE
    address = PC++;
    mem_fetch();
    relative_adderess = data_bus & 0x00FF;

    if (relative_adderess & 0x80) {   // if relative_adderess >= 128
        relative_adderess |= 0xFF00;  // the this is negative offset
    }

    return false;
}

bool MOS6502::IIX() {   // DONEADC
    address = PC++;
    mem_fetch();
    address = ((uint16_t)data_bus + (uint16_t)X) & 0x00FF;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address++;
    mem_fetch();
    address = ((((uint16_t)data_bus) << 8) & 0xFF00) | tmp_buff;

    return false;
}

bool MOS6502::IIY() {   // DONE
    address = PC++;
    mem_fetch();
    address = data_bus & 0x00FF;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address++;
    mem_fetch();
    address = (((((uint16_t)data_bus) << 8) & 0xFF00) | tmp_buff) + Y;

    if ((address & 0xFF00) != (((uint16_t)data_bus) << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::IND() {   // DONE
    address = PC++;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address = PC++;
    mem_fetch();
    tmp_buff = (((uint16_t)data_bus) << 8) | tmp_buff;

    address = tmp_buff;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;

    if (tmp_buff == 0x00FF) {    // Page boundary hardware bug
        address &= 0xFF00;
    } else { // Behave normally
        address++;
    }

    mem_fetch();
    address = (((uint16_t)data_bus) << 8) | tmp_buff;

    return false;
}


/********************************************************
 *                   INSTRUCTION SET                    *
 ********************************************************/
bool MOS6502::ADC() {   // DONE
    mem_fetch();

    // add is done in 16bit mode to catch the carry bit
    tmp_buff = (uint16_t)A + (uint16_t)data_bus + (read_flag(C) ? 0x0001 : 0x0000);

    set_flag(C, tmp_buff > 0x00FF);                 // Set the carry bit
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);     // Set Zero bit
    // Set Overflow bit
    set_flag(O, (~((uint16_t)(A ^ data_bus) & 0x00FF) & ((uint16_t)A ^ tmp_buff) & 0x0080));
    set_flag(N, tmp_buff & 0x80);                   // Set the Negative bit

    A = tmp_buff & 0x00FF;

    return true;
}

bool MOS6502::AND() {   // DONE
    mem_fetch();
    A = A & data_bus;

    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);
    return true;
}

bool MOS6502::ASL() {   // DONE
    mem_fetch();
    tmp_buff = ((uint16_t)data_bus) << 1;

    set_flag(C, (tmp_buff & 0xFF00) > 0);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x00);
    set_flag(N, tmp_buff & 0x80);

    if (accumulator_addressing) {
        A = tmp_buff & 0x00FF;
        accumulator_addressing = false;
    } else {
        data_bus = tmp_buff & 0x00FF;
        mem_write();
    }

    return false;
}

bool MOS6502::BCC() {   // DONE
    if (read_flag(C) == false) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::BCS() {   // DONE
    if (read_flag(C)) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::BEQ() {   // DONE
    if (read_flag(Z)) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::BIT() {   // DONE
    mem_fetch();
    tmp_buff = A & data_bus;

    set_flag(Z, (tmp_buff & 0x00FF) == 0x00);
    set_flag(N, data_bus & (1 << 7));
    set_flag(O, data_bus & (1 << 6));

    return false;
}

bool MOS6502::BMI() {   // DONE
    if (read_flag(N)) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::BNE() {   // DONE
    if (read_flag(Z) == false) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::BPL() {   // DONE
    if (read_flag(N) == false) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::BRK() {   // DONE
    PC++;

    set_flag(I, true);

    // Store PC on stack
    address = 0x0100 + S--;
    data_bus = (PC >> 8) & 0x00FF;
    mem_write();
    address = 0x0100 + S--;
    data_bus = PC & 0x00FF;
    mem_write();

    // Store P on stack
    set_flag(B, true);
    address = 0x0100 + S--;
    data_bus = P;
    mem_write();
    set_flag(B, false);

    // Set PC from address  $FFFE
    address = 0xFFFE;
    mem_fetch();
    tmp_buff = data_bus & 0x00FF;
    address = 0xFFFF;
    mem_fetch();
    PC = ((((uint16_t)data_bus) << 8) & 0xFF00) | tmp_buff;

    return false;
}

bool MOS6502::BVC() {   // DONE
    if (read_flag(O) == false) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::BVS() {   // DONE
    if (read_flag(O)) {
        cycles++;

        address = PC + relative_adderess;

        if ((address && 0xFF00) != (PC & 0xFF00)) {
            cycles++;
        }

        PC = address;
    }

    return false;
}

bool MOS6502::CLC() {   // DONE
    set_flag(C, false);
    return false;
}

bool MOS6502::CLD() {
    return true;
}

bool MOS6502::CLI() {
    return true;
}

bool MOS6502::CLV() {
    return true;
}

bool MOS6502::CMP() {
    return true;
}

bool MOS6502::CPX() {
    return true;
}

bool MOS6502::CPY() {
    return true;
}

bool MOS6502::DEC() {
    return true;
}

bool MOS6502::DEX() {
    X--;
    set_flag(Z, X == 0x00);
    set_flag(N, X & 0x80);

    return false;
}

bool MOS6502::DEY() {   // DONE
    Y--;
    set_flag(Z, Y == 0x00);
    set_flag(N, Y & 0x80);

    return false;
}

bool MOS6502::EOR() {
    return true;
}

bool MOS6502::INC() {
    return true;
}

bool MOS6502::INX() {
    return true;
}

bool MOS6502::INY() {
    return true;
}

bool MOS6502::JMP() {
    return true;
}

bool MOS6502::JSR() {
    return true;
}

bool MOS6502::LDA() {   // DONE
    mem_fetch();
    A = data_bus;

    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);

    return true;
}

bool MOS6502::LDX() {   // DONE
    mem_fetch();
    X = data_bus;

    set_flag(Z, X == 0x00);
    set_flag(N, X & 0x80);

    return true;
}

bool MOS6502::LDY() {   // DONE
    mem_fetch();
    Y = data_bus;

    set_flag(Z, Y == 0x00);
    set_flag(N, Y & 0x80);

    return true;
}

bool MOS6502::LSR() {
    return true;
}

bool MOS6502::NOP() {
    return true;
}

bool MOS6502::ORA() {
    return true;
}

bool MOS6502::PHA() {
    return true;
}

bool MOS6502::PHP() {
    return true;
}

bool MOS6502::PLA() {
    return true;
}

bool MOS6502::PLP() {
    return true;
}

bool MOS6502::ROL() {
    return true;
}

bool MOS6502::ROR() {
    return true;
}

bool MOS6502::RTI() {
    return true;
}

bool MOS6502::RTS() {
    return true;
}

bool MOS6502::SBC() {
    return true;
}

bool MOS6502::SEC() {
    return true;
}

bool MOS6502::SED() {
    return true;
}

bool MOS6502::SEI() {
    return true;
}

bool MOS6502::STA() {
    return true;
}

bool MOS6502::STX() {   // DONE
    data_bus = X;
    mem_write();

    return false;
}

bool MOS6502::STY() {   // DONE
    data_bus = Y;
    mem_write();

    return false;
}

bool MOS6502::TAX() {
    return true;
}

bool MOS6502::TAY() {
    return true;
}

bool MOS6502::TSX() {
    return true;
}

bool MOS6502::TXA() {
    return true;
}

bool MOS6502::TXS() {
    return true;
}

bool MOS6502::TYA() {
    return true;
}

bool MOS6502::XXX() {
    return true;
}
