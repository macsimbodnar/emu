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
        P |= flag;
    } else {
        P &= ~flag;
    }
}


bool MOS6502::read_flag(const status_flag_t flag) {
    return (P & flag);
}


void MOS6502::mem_read() {
    if (accumulator_addressing) {
        data_bus = A;
    } else {
        bus->access(address, access_mode_t::READ, data_bus);
    }
}

void MOS6502::mem_write() {
    if (accumulator_addressing) {
        A = data_bus;
    } else {
        bus->access(address, access_mode_t::WRITE, data_bus);
    }
}


void MOS6502::clock() {
    cycles = 0;

    if (cycles == 0) {

        // Reset the ACCUMULATOR addressing mode if set
        accumulator_addressing = false;

        address = PC++;

        mem_read();
        opcode = data_bus;

        // TEST
        PC_executed = address;

        if (opcode_table[opcode].instruction_bytes > 1) {
            bus->access(PC_executed + 1, access_mode_t::READ, arg1);
        }

        if (opcode_table[opcode].instruction_bytes > 2) {
            bus->access(PC_executed + 2, access_mode_t::READ, arg2);
        }

        // TEST END

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

    cycles--;
}


void MOS6502::reset() {
    // Reset registers
    A = 0x00;
    X = 0x00;
    Y = 0x00;

    S = 0xFD;
    P = 0x24;

    // Read from fix mem address to jump to programmable location
    address = 0xFFFC;
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address++;
    mem_read();
    PC = (((uint16_t)data_bus) << 8) | tmp_buff;

    // Clear helpers
    relative_adderess = 0x0000;
    address = 0x0000;
    data_bus = 0x00;
    accumulator_addressing = false;

    cycles = 6;
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
        mem_read();
        tmp_buff = data_bus & 0x00FF;
        address++;
        mem_read();
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
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address++;
    mem_read();
    PC = (((uint16_t)data_bus) << 8) | tmp_buff;

    cycles = 8;
}


p_state_t MOS6502::get_status() {
    return {A,
            X,
            Y,
            S,
            P,
            PC,
            opcode_table[opcode].name,
            opcode,
            opcode_table[opcode].instruction_bytes,
            data_bus,
            address,
            relative_adderess,
            tmp_buff,
            cycles,
            opcode_table[opcode].cycles,
            PC_executed,
            arg1,
            arg2,
            0};
}


void MOS6502::set_PC(uint16_t address) {
    PC = address;
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
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address = PC++;
    mem_read();
    address = (((uint16_t)data_bus) << 8) | tmp_buff;
    return false;
}

bool MOS6502::ZPI() {   // DONE
    address = PC++;
    mem_read();
    address = data_bus & 0x00FF;
    return false;
}

bool MOS6502::ZPX() {   // DONE
    address = PC++;
    mem_read();
    address = (data_bus + X) & 0x00FF;
    return false;
}

bool MOS6502::ZPY() {   // DONE
    address = PC++;
    mem_read();
    address = (data_bus + Y) & 0x00FF;
    return false;
}

bool MOS6502::ABX() {   // DONE
    address = PC++;
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address = PC++;
    mem_read();
    address = ((((uint16_t)data_bus) << 8) | tmp_buff) + X;

    if ((address & 0xFF00) != (((uint16_t)data_bus) << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::ABY() {   // DONE
    address = PC++;
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address = PC++;
    mem_read();
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
    mem_read();
    relative_adderess = data_bus & 0x00FF;

    if (relative_adderess & 0x80) {   // if relative_adderess >= 128
        relative_adderess |= 0xFF00;  // the this is negative offset
    }

    return false;
}

bool MOS6502::IIX() {   // DONE

    // TODO(max): fix, use only tmp
    // address = PC++;
    // mem_read();
    // address = ((uint16_t)data_bus + (uint16_t)X) & 0x00FF;
    // mem_read();
    // tmp_buff = data_bus & 0x00FF;
    // address++;
    // mem_read();
    // address = ((((uint16_t)data_bus) << 8) & 0xFF00) | tmp_buff;

    address = PC++;
    mem_read();
    tmp_buff = data_bus;

    address = (uint16_t)(tmp_buff + (uint16_t)X) & 0x00FF;
    mem_read();
    uint16_t lo = data_bus;

    address = (uint16_t)(tmp_buff + (uint16_t)X + 1) & 0x00FF;
    mem_read();
    uint16_t hi = data_bus;

    address = (hi << 8) | lo;

    return false;
}

bool MOS6502::IIY() {   // DONE
    // TODO(max): fix this with tmp only
    // address = PC++;
    // mem_read();
    // address = data_bus & 0x00FF;
    // mem_read();
    // tmp_buff = data_bus & 0x00FF;
    // address++;
    // mem_read();
    // address = (((((uint16_t)data_bus) << 8) & 0xFF00) | tmp_buff) + Y;

    // if ((address & 0xFF00) != (((uint16_t)data_bus) << 8)) {
    //     return true;
    // }

    address = PC++;
    mem_read();
    tmp_buff = data_bus;

    address = tmp_buff & 0x00FF;
    mem_read();
    uint16_t lo = data_bus;

    address = (tmp_buff + 1) & 0x00FF;
    mem_read();
    uint16_t hi = data_bus;

    address = (hi << 8) | lo;
    address += Y;

    if ((address & 0xFF00) != (hi << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::IND() {   // DONE
    // TODO(max): fix this
    // address = PC++;
    // mem_read();
    // tmp_buff = data_bus & 0x00FF;
    // address = PC++;
    // mem_read();
    // tmp_buff = (((uint16_t)data_bus) << 8) | tmp_buff;

    // address = tmp_buff;
    // mem_read();
    // tmp_buff = data_bus & 0x00FF;

    // if (tmp_buff == 0x00FF) {    // Page boundary hardware bug
    //     address &= 0xFF00;
    // } else { // Behave normally
    //     address++;
    // }

    // mem_read();
    // address = (((uint16_t)data_bus) << 8) | tmp_buff;

    address = PC++;
    mem_read();
    uint16_t lo = data_bus;

    address = PC++;
    mem_read();
    uint16_t hi = data_bus;

    tmp_buff = (hi << 8) | lo;

    if (lo == 0x00FF) { // Page boundary hardware bug
        address = tmp_buff;
        mem_read();
        lo = data_bus;

        address = tmp_buff & 0xFF00;
        mem_read();
        hi = data_bus;

        address = (hi << 8) | lo;
    } else {
        address = tmp_buff;
        mem_read();
        lo = data_bus;

        address = tmp_buff + 1;
        mem_read();
        hi = data_bus;

        address = (hi << 8) | lo;
    }

    return false;
}


/********************************************************
 *                   INSTRUCTION SET                    *
 ********************************************************/
bool MOS6502::ADC() {   // DONE
    mem_read();

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
    mem_read();
    A = A & data_bus;

    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);
    return true;
}

bool MOS6502::ASL() {   // DONE
    mem_read();
    tmp_buff = ((uint16_t)data_bus) << 1;

    set_flag(C, (tmp_buff & 0xFF00) > 0);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    data_bus = tmp_buff & 0x00FF;
    mem_write();

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
    mem_read();
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
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address = 0xFFFF;
    mem_read();
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

bool MOS6502::CLD() {   //DONE
    set_flag(D, false);
    return false;
}

bool MOS6502::CLI() {   // DONE
    set_flag(I, false);
    return false;
}

bool MOS6502::CLV() {   // DONE
    set_flag(O, false);
    return false;
}

bool MOS6502::CMP() {   // DONE
    mem_read();
    tmp_buff = (uint16_t)A - (uint16_t)data_bus;
    set_flag(C, A >= data_bus);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    return true;
}

bool MOS6502::CPX() {   // DONE
    mem_read();
    tmp_buff = (uint16_t)X - (uint16_t)data_bus;
    set_flag(C, X >= data_bus);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    return false;
}

bool MOS6502::CPY() {   // DONE
    mem_read();
    tmp_buff = (uint16_t)Y - (uint16_t)data_bus;
    set_flag(C, Y >= data_bus);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    return false;
}

bool MOS6502::DEC() {   // DONE
    mem_read();
    tmp_buff = data_bus - 1;
    data_bus = tmp_buff & 0x00FF;
    mem_write();
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    return false;
}

bool MOS6502::DEX() {   // DONE
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

bool MOS6502::EOR() {   // DONE
    mem_read();
    A = data_bus ^ A;
    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);
    return true;
}

bool MOS6502::INC() {   // DONE
    mem_read();
    tmp_buff = data_bus + 1;
    data_bus = tmp_buff & 0x00FF;
    mem_write();
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    return false;
}

bool MOS6502::INX() {   // DONE
    X++;
    set_flag(Z, X == 0x00);
    set_flag(N, X & 0x80);

    return false;
}

bool MOS6502::INY() {   // DONE
    Y++;
    set_flag(Z, Y == 0x00);
    set_flag(N, Y & 0x80);

    return false;
}

bool MOS6502::JMP() {   // DONE
    PC = address;
    return false;
}

bool MOS6502::JSR() {   // DONE
    PC--;

    tmp_buff = address;

    data_bus = (PC >> 8) & 0x00FF;
    address = 0x0100 + S--;
    mem_write();

    data_bus = PC & 0x00FF;
    address = 0x0100 + S--;
    mem_write();

    address = tmp_buff;
    PC = address;

    return false;
}

bool MOS6502::LDA() {   // DONE
    mem_read();
    A = data_bus;

    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);

    return true;
}

bool MOS6502::LDX() {   // DONE
    mem_read();
    X = data_bus;

    set_flag(Z, X == 0x00);
    set_flag(N, X & 0x80);

    return true;
}

bool MOS6502::LDY() {   // DONE
    mem_read();
    Y = data_bus;

    set_flag(Z, Y == 0x00);
    set_flag(N, Y & 0x80);

    return true;
}

bool MOS6502::LSR() {   // DONE
    mem_read();

    set_flag(C, data_bus & 0x01);

    tmp_buff = data_bus >> 1;

    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    // set_flag(N, tmp_buff & 0x0080);
    set_flag(N, false);

    data_bus = tmp_buff & 0x00FF;
    mem_write();

    return false;
}

bool MOS6502::NOP() {   // DONE
    // TODO(max): handle different nops
    switch (opcode) {
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
        return true;
        break;
    }

    return false;
}

bool MOS6502::ORA() {   // DONE
    mem_read();
    A = A | data_bus;
    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);
    return true;
}

bool MOS6502::PHA() {   // DONE
    data_bus = A;
    address = 0x0100 + S--;
    mem_write();
    return false;
}

bool MOS6502::PHP() {   // DONE
    set_flag(B, true);
    set_flag(U, true);

    data_bus = P;
    address = 0x0100 + S--;
    mem_write();
    set_flag(B, false);
    set_flag(U, false);

    return false;
}

bool MOS6502::PLA() {   // DONE
    S++;
    address = 0x0100 + S;
    mem_read();
    A = data_bus;
    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);

    return false;
}

bool MOS6502::PLP() {   // DONE
    S++;
    address = 0x0100 + S;
    mem_read();
    P = data_bus;
    set_flag(B, false);
    set_flag(U, true);
    return false;
}

bool MOS6502::ROL() {   // DONE
    mem_read();
    tmp_buff = (uint16_t)(data_bus << 1) | (read_flag(C) ? 1 : 0);
    set_flag(C, tmp_buff & 0xFF00);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    data_bus = tmp_buff & 0x00FF;
    mem_write();

    return false;
}

bool MOS6502::ROR() {   // DONE
    mem_read();
    tmp_buff = (uint16_t)((read_flag(C) ? 1 : 0) << 7) | (data_bus >> 1);
    set_flag(C, data_bus & 0x01);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(N, tmp_buff & 0x0080);

    data_bus = tmp_buff & 0x00FF;
    mem_write();

    return false;
}

bool MOS6502::RTI() {   // DONE
    S++;
    address = 0x0100 + S;
    mem_read();
    P = data_bus;
    P &= ~B;
    P &= ~U;

    S++;
    address = 0x0100 + S;
    mem_read();
    tmp_buff = (uint16_t)data_bus;
    S++;
    address = 0x0100 + S;
    mem_read();
    tmp_buff |= (uint16_t)data_bus << 8;

    PC = tmp_buff;

    return false;
}

bool MOS6502::RTS() {   // DONE
    S++;
    address = 0x0100 + S;
    mem_read();
    tmp_buff = (uint16_t)data_bus;
    S++;
    address = 0x0100 + S;
    mem_read();
    tmp_buff |= (uint16_t)data_bus << 8;

    PC = tmp_buff;
    PC++;

    return false;
}

// A = A - M - (1 - C)  ->  A = A + -1 * (M - (1 - C))  ->  A = A + (-M + 1 + C)
bool MOS6502::SBC() {   // DONE
    mem_read();

    uint16_t tv = (uint16_t)data_bus ^ 0x00FF;
    tmp_buff = (uint16_t)A + tv + (read_flag(C) ? 0x0001 : 0x0000);
    set_flag(C, tmp_buff & 0xFF00);
    set_flag(Z, (tmp_buff & 0x00FF) == 0x0000);
    set_flag(O, (tmp_buff ^ (uint16_t)A) & (tmp_buff ^ tv) & 0x0080);
    set_flag(N, tmp_buff & 0x0080);
    A = tmp_buff & 0x00FF;

    return true;
}

bool MOS6502::SEC() {   // DONE
    set_flag(C, true);
    return false;
}

bool MOS6502::SED() {   // DONE
    set_flag(D, true);
    return false;
}

bool MOS6502::SEI() {   // DON
    set_flag(I, true);
    return false;
}

bool MOS6502::STA() {   // DONE
    data_bus = A;
    mem_write();

    return false;
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

bool MOS6502::TAX() {   // DONE
    X = A;
    set_flag(Z, X == 0x00);
    set_flag(N, X & 0x80);

    return false;
}

bool MOS6502::TAY() {   // DONE
    Y = A;
    set_flag(Z, Y == 0x00);
    set_flag(N, Y & 0x80);

    return false;
}

bool MOS6502::TSX() {   // DONE
    X = S;
    set_flag(Z, X == 0x00);
    set_flag(N, X & 0x80);

    return false;
}

bool MOS6502::TXA() {   // DONE
    A = X;
    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);

    return false;
}

bool MOS6502::TXS() {   // DONE
    S = X;
    return false;
}

bool MOS6502::TYA() {
    A = Y;
    set_flag(Z, A == 0x00);
    set_flag(N, A & 0x80);

    return false;
}

/********************************************************
 *                  ILLEGAL INST SET                    *
 ********************************************************/

bool MOS6502::LAX() {
    mem_read();
    A = data_bus;
    X = data_bus;

    set_flag(Z, X == 0x00);
    set_flag(N, X & 0x80);

    return true;
}


bool MOS6502::SAX() {
    data_bus = A & X;
    mem_write();

    return false;
}


bool MOS6502::DCP() {
    DEC();
    CMP();

    return false;
}


bool MOS6502::ISB() {
    INC();
    SBC();

    return false;
}


bool MOS6502::SLO() {
    ASL();
    ORA();

    return false;
}


bool MOS6502::XXX() {
    log_e("Executed illegal opcode");
    return false;
}
