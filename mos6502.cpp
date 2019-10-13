#include "mos6502.hpp"

#define MICROCODE(code) microcode_q.enqueue(([](MOS6502 * cpu) -> void { code }))


MOS6502::MOS6502(mem_access_callback mem_acc_clb, void *usr_data) :
    mem_access(mem_acc_clb), user_data(usr_data) {}


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
        // NOTE(max): intentionally not checking if function is nullptr
        mem_access(user_data, address_bus, access_mode_t::READ, data_bus);
    }
}

void MOS6502::mem_write() {
    if (accumulator_addressing) {
        A = data_bus;
    } else {
        // NOTE(max): intentionally not checking if function is nullptr
        mem_access(user_data, address_bus, access_mode_t::WRITE, data_bus);
    }
}


bool MOS6502::clock() {
    cycles++;

    if (microcode_q.is_empty()) {   // Fetch and decode next instruction
        accumulator_addressing = false;
        address_bus = PC++;
        mem_read();
        opcode = data_bus;

        // TEST
        PC_executed = address_bus;

        if (opcode_table[opcode].instruction_bytes > 1) {
            mem_access(user_data, PC_executed + 1, access_mode_t::READ, arg1);
        }

        if (opcode_table[opcode].instruction_bytes > 2) {
            mem_access(user_data, PC_executed + 2, access_mode_t::READ, arg2);
        } // TEST END

        (this->*opcode_table[opcode].addrmode)();
        (this->*opcode_table[opcode].operation)();

    } else {                        // Execute next microcode step
        micro_op_t micro_operation;

        if (microcode_q.dequeue(micro_operation)) {
            // Exec the microcode
            micro_operation(this);
        } else {
            log("Error dequeueing nex microcode instruction");
        }
    }

    // TEST
    if (microcode_q.is_empty()) {
        return true;
    }

    return false;
    // TEST END
}


void MOS6502::reset() {
    // Reset registers
    A = 0x00;
    X = 0x00;
    Y = 0x00;

    S = STACK_POINTER_DEFAULT;
    P = PROCESSOR_STATUS_DEFAULT;

    // Read from fix mem address to jump to programmable location
    address_bus = INITIAL_ADDRESS;
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address_bus++;
    mem_read();
    PC = (((uint16_t)data_bus) << 8) | tmp_buff;

    // Clear helpers
    relative_adderess = 0x0000;
    address_bus = 0x0000;
    data_bus = 0x00;
    accumulator_addressing = false;
    cycles = 7;
}


void MOS6502::irq() {   // Read from 0xFFFE
    if (read_flag(I) == false) {
        // Push PC on the stack
        // Write first the high because the stack decrease
        address_bus = STACK_OFFSET + S--;
        data_bus = (PC >> 8) & 0x00FF;
        mem_write();    // write high byte

        address_bus = STACK_OFFSET + S--;
        data_bus = PC & 0x00FF;
        mem_write();    // write low byte

        // Push status on stack
        set_flag(B, false);
        set_flag(U, true);
        set_flag(I, true);

        data_bus = P;
        address_bus = STACK_OFFSET + S--;
        mem_write();

        // Read new PC from the fixed location
        address_bus = 0xFFFE;
        mem_read();
        tmp_buff = data_bus & 0x00FF;
        address_bus++;
        mem_read();
        PC = (((uint16_t)data_bus) << 8) | tmp_buff;
    }
}


void MOS6502::nmi() {   // Read from 0xFFFA
    // Push PC on the stack
    // Write first the high because the stack decrease
    address_bus = STACK_OFFSET + S--;
    data_bus = (PC >> 8) & 0x00FF;
    mem_write();    // write high byte

    address_bus = STACK_OFFSET + S--;
    data_bus = PC & 0x00FF;
    mem_write();    // write low byte

    // Push status on stack
    set_flag(B, false);
    set_flag(U, true);
    set_flag(I, true);

    data_bus = P;
    address_bus = STACK_OFFSET + S--;
    mem_write();

    // Read new PC from the fixed location
    address_bus = 0xFFFA;
    mem_read();
    tmp_buff = data_bus & 0x00FF;
    address_bus++;
    mem_read();
    PC = (((uint16_t)data_bus) << 8) | tmp_buff;
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
            address_bus,
            relative_adderess,
            tmp_buff,
            0,  // cycles
            opcode_table[opcode].cycles,
            PC_executed,
            arg1,
            arg2,
            cycles};
}


void MOS6502::set_PC(uint16_t address) {
    PC = address;
}


void MOS6502::set_log_callback(log_callback log_clb) {
    log_func = log_clb;
}


void MOS6502::log(const std::string &msg) {
    if (log_func) {
        log_func(msg);
    }
}


/********************************************************
 *                  ADDRESSING MODES                    *
 ********************************************************/
void MOS6502::ACC() {   // DONE
    accumulator_addressing = true;
}

void MOS6502::IMM() {   // DONE
    address_bus = PC++;

    // TODO(max): this one after decode the opcode start to execute it
}

void MOS6502::ABS() {   // DONE
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus & 0x00FF;
        cpu->address_bus = cpu->PC++;
    );

    MICROCODE(
        cpu->mem_read();
        cpu->address_bus = (((uint16_t)cpu->data_bus) << 8) | cpu->tmp_buff;
    );
}

void MOS6502::ZPI() {   // DONE
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->address_bus = cpu->data_bus & 0x00FF;
    );
}

void MOS6502::ZPX() {   // DONE
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->address_bus = (cpu->data_bus + cpu->X) & 0x00FF;
    );
}

void MOS6502::ZPY() {   // DONE
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->address_bus = (cpu->data_bus + cpu->Y) & 0x00FF;
    );
}

void MOS6502::ABX() {   // DONE
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus & 0x00FF;
        cpu->address_bus = cpu->PC++;
    );

    MICROCODE(
        cpu->mem_read();
        cpu->address_bus = ((((uint16_t)cpu->data_bus) << 8) | cpu->tmp_buff) + cpu->X;
    );

    // if ((address & 0xFF00) != (((uint16_t)data_bus) << 8)) {
    //     // TODO() fix this with reasonable code
    //     MICROCODE(
    //         asm("nop");
    //     );
    // }
}

void MOS6502::ABY() {   // DONE
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus & 0x00FF;
        cpu->address_bus = cpu->PC++;
    );

    MICROCODE(
        cpu->mem_read();
        cpu->address_bus = ((((uint16_t)cpu->data_bus) << 8) | cpu->tmp_buff) + cpu->Y;
    );

    // if ((address & 0xFF00) != (((uint16_t)data_bus) << 8)) {
    //     // TODO() fix this with reasonable code
    //     MICROCODE(
    //         asm("nop");
    //     );
    // }
}

void MOS6502::IMP() {   // DONE
    asm("nop");
}

void MOS6502::REL() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->relative_adderess = cpu->data_bus & 0x00FF;
        if (cpu->relative_adderess & 0x80) {   /* if relative_adderess >= 128 */
            cpu->relative_adderess |= 0xFF00;  /* the this is negative offset */
        }
    );
// *INDENT-ON*
}

void MOS6502::IIX() {   // DONE

    // TODO(max): fix, use only tmp
    // address = PC++;
    // mem_read();
    // address = ((uint16_t)data_bus + (uint16_t)X) & 0x00FF;
    // mem_read();
    // tmp_buff = data_bus & 0x00FF;
    // address++;
    // mem_read();
    // address = ((((uint16_t)data_bus) << 8) & 0xFF00) | tmp_buff;
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus;

        cpu->address_bus = (uint16_t)(cpu->tmp_buff + (uint16_t)cpu->X) & 0x00FF;
    );

    MICROCODE(
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        cpu->address_bus = (uint16_t)(cpu->tmp_buff + (uint16_t)cpu->X + 1) & 0x00FF;
    );

    MICROCODE(
        cpu->mem_read();
        cpu->hi = cpu->data_bus;

        cpu->address_bus = (cpu->hi << 8) | cpu->lo;
    );
}

void MOS6502::IIY() {   // DONE
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
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus;

        cpu->address_bus = cpu->tmp_buff & 0x00FF;
    );

    MICROCODE(
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        cpu->address_bus = (cpu->tmp_buff + 1) & 0x00FF;
    );

    MICROCODE(
        cpu->mem_read();
        cpu->hi = cpu->data_bus;

        cpu->address_bus = (cpu->hi << 8) | cpu->lo;
        cpu->address_bus += cpu->Y;
    );
    // if ((address & 0xFF00) != (hi << 8)) {
    // TODO() fix this with reasonable code
    //     MICROCODE(
    //         asm("nop");
    //     );
    // }
}

void MOS6502::IND() {   // DONE
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

// *INDENT-OFF*
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        if (cpu->lo == 0x00FF) {
            cpu->page_boundary_crossed = true;
        } else {
            cpu->page_boundary_crossed = false;
        }

        cpu->address_bus = cpu->PC++;
    );
// *INDENT-ON*

    MICROCODE(
        cpu->mem_read();
        cpu->hi = cpu->data_bus;

        cpu->tmp_buff = (cpu->hi << 8) | cpu->lo;

        cpu->address_bus = cpu->tmp_buff;
    );

// *INDENT-OFF*
    MICROCODE(
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        if (cpu->page_boundary_crossed) { /* Page boundary hardware bug */
            cpu->address_bus = cpu->tmp_buff & 0xFF00;
        } else {
            cpu->address_bus = cpu->tmp_buff + 1;
        }
    );
// *INDENT-ON*

    MICROCODE(
        cpu->mem_read();
        cpu->hi = cpu->data_bus;

        cpu->address_bus = (cpu->hi << 8) | cpu->lo;
    );
}


/********************************************************
 *                   INSTRUCTION SET                    *
 ********************************************************/
void MOS6502::ADC() {   // DONE
    MICROCODE(
        cpu->mem_read();

        /* add is done in 16bit mode to catch the carry bit */
        cpu->tmp_buff = (uint16_t)cpu->A + (uint16_t)cpu->data_bus + (cpu->read_flag(
                            MOS6502::C) ? 0x0001 : 0x0000);

        cpu->set_flag(MOS6502::C, cpu->tmp_buff > 0x00FF);                 /* Set the carry bit */
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);     /* Set Zero bit */
        /* Set Overflow bit */
        cpu->set_flag(MOS6502::O, (~((uint16_t)(cpu->A ^ cpu->data_bus) & 0x00FF) &
                                   ((uint16_t)cpu->A ^ cpu->tmp_buff) & 0x0080));
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x80);                   /* Set the Negative bit */

        cpu->A = cpu->tmp_buff & 0x00FF;
    );
}

void MOS6502::AND() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->A = cpu->A & cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::ASL() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = ((uint16_t)cpu->data_bus) << 1;

        cpu->set_flag(MOS6502::C, (cpu->tmp_buff & 0xFF00) > 0);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::BCC() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(C) == false) {
            /* cycles++; */
            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                 cycles++;
            }
            */
            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::BCS() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(C)) {
            /* cycles++; */
            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                cycles++;
            }
            */
            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::BEQ() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(Z)) {
            /* cycles++; */

            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                cycles++;
            }
            */

            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::BIT() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = cpu->A & cpu->data_bus;

        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x00);
        cpu->set_flag(MOS6502::N, cpu->data_bus & (1 << 7));
        cpu->set_flag(MOS6502::O, cpu->data_bus & (1 << 6));
    );
}

void MOS6502::BMI() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(MOS6502::N)) {
            /* cycles++; */

            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                cycles++;
            }
            */

            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::BNE() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(MOS6502::Z) == false) {
            /* cycles++; */

            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                cycles++;
            }
            */

            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::BPL() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(MOS6502::N) == false) {
            /* cycles++; */

            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                cycles++;
            }
            */

            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::BRK() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away), increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
    );


    // TICK(3): Push PC H on stack, decrement S
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->data_bus = (cpu->PC >> 8) & 0x00FF;
        cpu->mem_write();
    );

    // TICK(4): Push PC L on stack, decrement S
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->data_bus = cpu->PC & 0x00FF;
        cpu->mem_write();
    );

    // TICK(5): Push P on stack (with B flag set), decrement S
    MICROCODE(
        /* Store P on stack */
        cpu->set_flag(MOS6502::B, true);
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->data_bus = cpu->P;
        cpu->mem_write();
        /* TODO(max): verify if this should be false after push */
        cpu->set_flag(MOS6502::B, false);
    );

    // TICK(6): Fetch PC L from 0xFFFE
    MICROCODE(
        cpu->address_bus = BRK_PCL;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus & 0x00FF;
    );

    // TICK(7): Fetch PC H from 0xFFFF
    MICROCODE(
        cpu->address_bus = BRK_PCH;
        cpu->mem_read();
        cpu->PC = ((((uint16_t)cpu->data_bus) << 8) & 0xFF00) | cpu->tmp_buff;
    );
}

void MOS6502::BVC() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(MOS6502::O) == false) {
            /* cycles++; */

            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                cycles++;
            }
            */

            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::BVS() {   // DONE
// *INDENT-OFF*
    MICROCODE(
        if (cpu->read_flag(MOS6502::O)) {
            /* cycles++; */

            cpu->address_bus = cpu->PC + cpu->relative_adderess;
            /*
            if ((address & 0xFF00) != (PC & 0xFF00)) {
                cycles++;
            }
            */

            cpu->PC = cpu->address_bus;
        }
    );
// *INDENT-ON*
}

void MOS6502::CLC() {   // DONE
    MICROCODE(
        cpu->set_flag(MOS6502::C, false);
    );
}

void MOS6502::CLD() {   //DONE
    MICROCODE(
        cpu->set_flag(MOS6502::D, false);
    );
}

void MOS6502::CLI() {   // DONE
    MICROCODE(
        cpu->set_flag(MOS6502::I, false);
    );
}

void MOS6502::CLV() {   // DONE
    MICROCODE(
        cpu->set_flag(MOS6502::O, false);
    );
}

void MOS6502::CMP() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)cpu->A - (uint16_t)cpu->data_bus;
        cpu->set_flag(MOS6502::C, cpu->A >= cpu->data_bus);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );
}

void MOS6502::CPX() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)cpu->X - (uint16_t)cpu->data_bus;
        cpu->set_flag(MOS6502::C, cpu->X >= cpu->data_bus);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );
}

void MOS6502::CPY() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)cpu->Y - (uint16_t)cpu->data_bus;
        cpu->set_flag(MOS6502::C, cpu->Y >= cpu->data_bus);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );
}

void MOS6502::DEC() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus - 1;
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
    );

    MICROCODE(
        cpu->mem_write();
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );
}

void MOS6502::DEX() {   // DONE
    MICROCODE(
        cpu->X--;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::DEY() {   // DONE
    MICROCODE(
        cpu->Y--;
        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::EOR() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->A = cpu->data_bus ^ cpu->A;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::INC() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus + 1;
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
    );

    MICROCODE(
        cpu->mem_write();
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);

    );
}

void MOS6502::INX() {   // DONE
    MICROCODE(
        cpu->X++;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::INY() {   // DONE
    MICROCODE(
        cpu->Y++;
        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::JMP() {   // DONE
    MICROCODE(
        cpu->PC = cpu->address_bus;
    );
}

void MOS6502::JSR() {   // DONE
    // NOTE(max):   Clear the microcode because the JSR act different from normal
    //              abbsolute addressing mode so i need to remove the addressing mode code
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch low address byte, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus & 0x00FF;
    );

    // TICK(3): Internal operation (predecrement S?)
    MICROCODE(
        asm("nop");
    );

    // TICK(4): Push PC H on stack, decrement S
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->data_bus = (cpu->PC >> 8) & 0x00FF;
        cpu->mem_write();
    );

    // TICK(5): Push PC L on stack, decrement S
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->data_bus = cpu->PC & 0x00FF;
        cpu->mem_write();
    );

    // TICK(6): Copy low address byte to PC L, fetch high address byte to PC H
    MICROCODE(
        cpu->address_bus = cpu->PC;
        cpu->mem_read();
        cpu->PC = (((uint16_t)cpu->data_bus) << 8) | cpu->tmp_buff;
    );
}

void MOS6502::LDA() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->A = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::LDX() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->X = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::LDY() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->Y = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::LSR() {   // DONE
    MICROCODE(
        cpu->mem_read();

        cpu->set_flag(MOS6502::C, cpu->data_bus & 0x01);

        cpu->tmp_buff = cpu->data_bus >> 1;

        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        /* cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080); */
        cpu->set_flag(MOS6502::N, false);
    );

    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::NOP() {   // DONE
    // TODO(max): handle different nops
    // switch (opcode) {
    // case 0x1C:
    // case 0x3C:
    // case 0x5C:
    // case 0x7C:
    // case 0xDC:
    // case 0xFC:

    //     break;
    // }
    MICROCODE(
        asm("nop");
    );
}

void MOS6502::ORA() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->A = cpu->A | cpu->data_bus;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::PHA() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
    );

    // TICK(3): Push register on stack, decrement S
    MICROCODE(
        cpu->data_bus = cpu->A;
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->mem_write();
    );
}

void MOS6502::PHP() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
    );

    // TICK(3): Push register on stack, decrement S
    MICROCODE(
        cpu->set_flag(MOS6502::B, true);
        cpu->data_bus = cpu->P;
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->mem_write();
        cpu->set_flag(MOS6502::B, false);
    );
}

void MOS6502::PLA() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
    );

    // TICK(3): Increment S
    MICROCODE(
        cpu->S++;
    );

    // TICK(4): Pull register from stack
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S;
        cpu->mem_read();
        cpu->A = cpu->data_bus;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::PLP() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
    );

    // TICK(3): Increment S
    MICROCODE(
        cpu->S++;
    );

    // TICK(4): Pull register from stack
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S;
        cpu->mem_read();
        cpu->P = cpu->data_bus;
        cpu->set_flag(MOS6502::B, false);
        cpu->set_flag(MOS6502::U, true);
    );
}

void MOS6502::ROL() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)(cpu->data_bus << 1) | (cpu->read_flag(MOS6502::C) ? 1 : 0);
        cpu->set_flag(MOS6502::C, cpu->tmp_buff & 0xFF00);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::ROR() {   // DONE
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)((cpu->read_flag(MOS6502::C) ? 1 : 0) << 7) | (cpu->data_bus >> 1);
        cpu->set_flag(MOS6502::C, cpu->data_bus & 0x01);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::RTI() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
    );

    // TICK(3): Increment S
    MICROCODE(
        cpu->S++;
    );

    // TICK(4): Pull P from stack, increment S
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S;
        cpu->mem_read();
        cpu->P = cpu->data_bus;
        /* TODO(max): why this is not zero? */
        cpu->set_flag(MOS6502::U, true);
        cpu->S++;
    );

    // TICK(5): Pull PC L from stack, increment S
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S;
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)cpu->data_bus;
        cpu->S++;
    );

    // TICK(6): Pull PC H from stack
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S;
        cpu->mem_read();
        cpu->tmp_buff |= (uint16_t)cpu->data_bus << 8;

        cpu->PC = cpu->tmp_buff;
    );
}

void MOS6502::RTS() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
    );

    // TICK(3): Increment S
    MICROCODE(
        cpu->S++;
    );

    // TICK(4): Pull PC L from stack, increment S
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S;
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)cpu->data_bus;
        cpu->S++;
    );

    // TICK(5): Pull PC H from stack
    MICROCODE(
        cpu->address_bus = STACK_OFFSET + cpu->S;
        cpu->mem_read();
        cpu->tmp_buff |= (uint16_t)cpu->data_bus << 8;
        cpu->PC = cpu->tmp_buff;
    );

    // TICK(6): Increment PC
    MICROCODE(
        cpu->PC++;
    );
}

// cpu->A = cpu->A - M - (1 - MOS6502::C)  ->  cpu->A = cpu->A + -1 * (M - (1 - MOS6502::C))  ->  cpu->A = cpu->A + (-M + 1 + MOS6502::C)
void MOS6502::SBC() {   // DONE
    MICROCODE(
        cpu->mem_read();

        uint16_t tv = (uint16_t)cpu->data_bus ^ 0x00FF;
        cpu->tmp_buff = (uint16_t)cpu->A + tv + (cpu->read_flag(MOS6502::C) ? 0x0001 : 0x0000);
        cpu->set_flag(MOS6502::C, cpu->tmp_buff & 0xFF00);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::O, (cpu->tmp_buff ^ (uint16_t)cpu->A) & (cpu->tmp_buff ^ tv) & 0x0080);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
        cpu->A = cpu->tmp_buff & 0x00FF;
    );
}

void MOS6502::SEC() {   // DONE
    MICROCODE(
        cpu->set_flag(MOS6502::C, true);
    );
}

void MOS6502::SED() {   // DONE
    MICROCODE(
        cpu->set_flag(D, true);
    );
}

void MOS6502::SEI() {   // DON
    MICROCODE(
        cpu->set_flag(I, true);
    );
}

void MOS6502::STA() {   // DONE
    MICROCODE(
        cpu->data_bus = cpu->A;
        cpu->mem_write();
    );
}

void MOS6502::STX() {   // DONE
    MICROCODE(
        cpu->data_bus = cpu->X;
        cpu->mem_write();
    );
}

void MOS6502::STY() {   // DONE
    MICROCODE(
        cpu->data_bus = cpu->Y;
        cpu->mem_write();
    );
}

void MOS6502::TAX() {   // DONE
    MICROCODE(
        cpu->X = cpu->A;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::TAY() {   // DONE
    MICROCODE(
        cpu->Y = cpu->A;
        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::TSX() {   // DONE
    MICROCODE(
        cpu->X = cpu->S;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::TXA() {   // DONE
    MICROCODE(
        cpu->A = cpu->X;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::TXS() {   // DONE
    MICROCODE(
        cpu->S = cpu->X;
    );
}

void MOS6502::TYA() {
    MICROCODE(
        cpu->A = cpu->Y;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

/********************************************************
 *                  ILLEGAL INST SET                    *
 ********************************************************/

void MOS6502::LAX() {
    MICROCODE(
        cpu->mem_read();
        cpu->A = cpu->data_bus;
        cpu->X = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}


void MOS6502::SAX() {
    MICROCODE(
        cpu->data_bus = cpu->A & cpu->X;
        cpu->mem_write();
    );
}


void MOS6502::DCP() {
    DEC();
    CMP();
}


void MOS6502::ISB() {
    INC();
    SBC();
}


void MOS6502::SLO() {
    ASL();
    ORA();
}


void MOS6502::RLA() {
    ROL();
    AND();
}


void MOS6502::SRE() {
    LSR();
    EOR();
}


void MOS6502::RRA() {
    ROR();
    ADC();
}


void MOS6502::XXX() {
    log("Executed illegal opcode");
}
