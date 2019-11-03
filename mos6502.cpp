#include "mos6502.hpp"

#define MICROCODE(code) microcode_q.enqueue(([](MOS6502 * cpu) -> void { code }))
#define MICROCODE_IN_FRONT(code) microcode_q.insert_in_front(([](MOS6502 * cpu) -> void { code }))
#define ADDRESS(hi, lo) (static_cast<uint16_t>((static_cast<uint16_t>(hi) << 8) | (lo)))
// #define ADDRESS(hi, lo) (static_cast<uint16_t>(256U * hi + lo))
// Combine the high bits of first argument with low bits of second argument
#define COMBINE(hi, lo) ((hi & 0x00FF) | (lo & 0xFF00))


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
        instruction = &(opcode_table[opcode]);

        // TEST
        PC_executed = address_bus;

        if (instruction->instruction_bytes > 1) {
            mem_access(user_data, PC_executed + 1, access_mode_t::READ, arg1);
        }

        if (instruction->instruction_bytes > 2) {
            mem_access(user_data, PC_executed + 2, access_mode_t::READ, arg2);
        } // TEST END

        (this->*instruction->addrmode)();
        (this->*instruction->operation)();

    } else {                        // Execute next microcode step

        // if we are in accumulator addressing mode we read from accumulator
        // and not from memory so all reads and writes can be executed now
        do {
            micro_op_t micro_operation;

            if (microcode_q.dequeue(micro_operation)) {
                // Exec the microcode
                micro_operation(this);
            } else {
                log("Error dequeueing nex microcode instruction");
            }

        } while (!microcode_q.is_empty() && accumulator_addressing);
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


bool MOS6502::is_read_instruction() {
    auto current_op = instruction->operation;

    if (
        current_op == &MOS6502::LDA || current_op == &MOS6502::LDX ||
        current_op == &MOS6502::LDY || current_op == &MOS6502::EOR ||
        current_op == &MOS6502::AND || current_op == &MOS6502::ORA ||
        current_op == &MOS6502::ADC || current_op == &MOS6502::SBC ||
        current_op == &MOS6502::CMP || current_op == &MOS6502::BIT ||
        current_op == &MOS6502::LAX || current_op == &MOS6502::NOP) {

        return true;
    }

    return false;
}


/********************************************************
 *                  ADDRESSING MODES                    *
 ********************************************************/
void MOS6502::ACC() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        // Make mem read and write not from memory but from accumulator register
        cpu->accumulator_addressing = true;
    );
}

void MOS6502::IMM() {
    // TICK(1): Fetch opcode, increment PC

    // TODO(max): The PC should be incremented at the same cycle that the mem_read!
    address_bus = PC++;
}

void MOS6502::ABS() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch low byte of address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;
        cpu->address_bus = cpu->PC++;
    );

    // TICK(3): Fetch high byte of address, increment PC
    MICROCODE(
        cpu->mem_read();
        cpu->address_bus = ADDRESS(cpu->data_bus, cpu->lo);
    );
}

void MOS6502::ZPI() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->address_bus = cpu->data_bus & 0x00FF;
    );
}

void MOS6502::ZPX() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
    );

    // TICK(3): Read from address, add index register to it
    MICROCODE(
        cpu->address_bus = cpu->data_bus & 0x00FF;
        cpu->mem_read();
        cpu->address_bus = (cpu->address_bus + cpu->X) & 0x00FF;
    );
}

void MOS6502::ZPY() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
    );

    // TICK(3): Read from address, add index register to it
    MICROCODE(
        cpu->address_bus = cpu->data_bus & 0x00FF;
        cpu->mem_read();
        cpu->address_bus = (cpu->address_bus + cpu->Y) & 0x00FF;
    );
}

void MOS6502::ABX() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch low byte of address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;
    );

    // TICK(3): Fetch high byte of address, add index register to low address byte, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->hi = cpu->data_bus;
        cpu->tmp_buff = cpu->lo + cpu->X;

        cpu->lo = cpu->tmp_buff & 0x00FF;
    );

// *INDENT-OFF*
    // TICK(4): Read from effective address, fix the high byte of effective address
    MICROCODE(
        cpu->address_bus = ADDRESS(cpu->hi, cpu->lo);

        if (cpu->tmp_buff & 0xFF00) {   /* Page was crossed */
            cpu->mem_read();
            cpu->address_bus += 0x0100;
        } else {
            if (cpu->is_read_instruction()) {
                /*                                  NOTE(max):
                 *    Check page crossing: if no page boundary was crossed then the address is correct
                 *    and the re-read can be skiped in the Absolute indexed addressing Read instructions
                 *    (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT, LAX, LAE, SHS, NOP)
                 */

                // Exec immediately the next instruction
                if (!cpu->microcode_q.is_empty()) {
                    micro_op_t micro_operation;
                    cpu->microcode_q.dequeue(micro_operation);
                    micro_operation(cpu);
                }
            } else {
                cpu->mem_read();
            }
        }
    );
// *INDENT-ON*
}

void MOS6502::ABY() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch low byte of address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;
    );

    // TICK(3): Fetch high byte of address, add index register to low address byte, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->hi = cpu->data_bus;
        cpu->tmp_buff = cpu->lo + cpu->Y;

        cpu->lo = cpu->tmp_buff & 0x00FF;
    );

// *INDENT-OFF*
    // TICK(4): Read from effective address, fix the high byte of effective address
    MICROCODE(
        cpu->address_bus = ADDRESS(cpu->hi, cpu->lo);

        if (cpu->tmp_buff & 0xFF00) {   /* Page was crossed */
            cpu->mem_read();
            cpu->address_bus += 0x0100;
        } else {
            if (cpu->is_read_instruction()) {
                /*                                  NOTE(max):
                 *    Check page crossing: if no page boundary was crossed then the address is correct
                 *    and the re-read can be skiped in the Absolute indexed addressing Read instructions
                 *    (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT, LAX, LAE, SHS, NOP)
                 */

                // Exec immediately the next instruction
                if (!cpu->microcode_q.is_empty()) {
                    micro_op_t micro_operation;
                    cpu->microcode_q.dequeue(micro_operation);
                    micro_operation(cpu);
                }
            } else {
                cpu->mem_read();
            }
        }
    );
// *INDENT-ON*
}

void MOS6502::IMP() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
    );
}

void MOS6502::REL() {
    // NOTE(max):   This addressing mode is specific for Branching,
    //              all cases are handled inside the instruction
    asm("nop");
}

void MOS6502::IIX() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch pointer address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;
    );

    // TICK(3): Read from the address, add X to it
    MICROCODE(
        cpu->address_bus = static_cast<uint16_t>(cpu->lo) & 0x00FF;
        cpu->mem_read();
        cpu->address_bus = (cpu->lo + cpu->X) & 0x00FF; /* No page crossing, discarding the carry */
    );

    // TICK(3): Fetch effective address low
    MICROCODE(
        cpu->mem_read();
        cpu->lo = cpu->data_bus;
    );

    // TICK(4): Fetch effective address high
    MICROCODE(
        cpu->address_bus = (cpu->address_bus + 1) & 0x00FF; /* No page crossing */
        cpu->mem_read();
        cpu->address_bus = ADDRESS(cpu->data_bus, cpu->lo);
    );
}

void MOS6502::IIY() {
    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch pointer address, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
    );

    // TICK(3): Fetch effective address low
    MICROCODE(
        cpu->address_bus = static_cast<uint16_t>(cpu->data_bus) & 0x00FF;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;
    );

// *INDENT-OFF*
    // TICK(4): Fetch effective address high, add Y to low byte of effective address
    MICROCODE(
        /* The effective address is always fetched from zero page */
        cpu->address_bus = (cpu->address_bus + 1) & 0x00FF; /* No page crossing */
        cpu->mem_read();
        cpu->hi = cpu->data_bus;

        cpu->tmp_buff = static_cast<uint16_t>(cpu->lo) + cpu->Y;

        cpu->address_bus = ADDRESS(cpu->hi, cpu->tmp_buff & 0x00FF);
    );

    // TICK(5):     Read from effective address, fix high byte of effective address
    MICROCODE(
        if (cpu->tmp_buff & 0xFF00) {   /* Page was crossed */
            cpu->mem_read();
            cpu->address_bus += 0x0100;
        } else {
            if (cpu->is_read_instruction()) {
                // NOTE(max):   Only in Read instructions (LDA, EOR, AND, ORA, ADC, SBC, CMP) 
                //              the next cycle will be executed only if boundary was crossed

                // Exec immediately the next instruction
                if (!cpu->microcode_q.is_empty()) {
                    micro_op_t micro_operation;
                    cpu->microcode_q.dequeue(micro_operation);
                    micro_operation(cpu);
                }
            } else {
                cpu->mem_read();
            }
        }
    );
// *INDENT-ON*
}

void MOS6502::IND() {
    // This is just a special JMP
    asm("nop");
}


/********************************************************
 *                   INSTRUCTION SET                    *
 ********************************************************/
void MOS6502::ADC() {
    // TICK(A + 1): Read from effective address
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

void MOS6502::AND() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();

        cpu->A = cpu->A & cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::ASL() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
    );

    // TICK(A + 2): Write the value back to effective address, and do the operation on it
    MICROCODE(
        cpu->mem_write();

        cpu->tmp_buff = ((uint16_t)cpu->data_bus) << 1;

        cpu->set_flag(MOS6502::C, (cpu->tmp_buff & 0xFF00) > 0);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    // TICK(A + 3): Write the new value to effective address
    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::BCC() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(C) == false) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::BCS() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(C)) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::BEQ() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(Z)) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::BIT() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = cpu->A & cpu->data_bus;

        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x00);
        cpu->set_flag(MOS6502::N, cpu->data_bus & (1 << 7));
        cpu->set_flag(MOS6502::O, cpu->data_bus & (1 << 6));
    );
}

void MOS6502::BMI() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(N)) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::BNE() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(Z) == false) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::BPL() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(N) == false) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::BRK() {
    // NOTE(max):   Clear the microcode_q because in this case the
    //              implied addressing mode is different
    microcode_q.clear();

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

void MOS6502::BVC() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(O) == false) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::BVS() {
    // TICK(1): Fetch opcode, increment PC

// *INDENT-OFF*
    // TICK(2): Fetch operand, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->lo = cpu->data_bus;

        /* if no branch taken just go with other instruction */ 
        if (cpu->read_flag(O)) {
           /* Branch Taken */

            // TICK(3): If branch is taken, add operand to PCL.
            cpu->MICROCODE(
                /* Read the memory after the instruction */
                cpu->address_bus = cpu->PC;
                cpu->mem_read();

                cpu->tmp_buff = cpu->PC + static_cast<int8_t>(cpu->lo);

                if ((cpu->tmp_buff & 0xFF00) != (cpu->PC & 0xFF00)) { /* page crossing */
                    // TICK(4): Fix PCH. If it did not change, increment PC.
                    cpu->MICROCODE(
                        cpu->address_bus = cpu->PC;
                        cpu->mem_read();

                        if (cpu->lo & 0x80) {   /* if relative_adderess >= 128 */
                            cpu->PC -= 0x0100;
                        } else {
                            cpu->PC += 0x0100;
                        }
                    );
                } 

                /* Set the low bits*/
                cpu->PC = COMBINE(cpu->tmp_buff, cpu->PC);
            );
        }
    );
// *INDENT-ON*
}

void MOS6502::CLC() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->set_flag(MOS6502::C, false);
    );
}

void MOS6502::CLD() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->set_flag(MOS6502::D, false);
    );
}

void MOS6502::CLI() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->set_flag(MOS6502::I, false);
    );
}

void MOS6502::CLV() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->set_flag(MOS6502::O, false);
    );
}

void MOS6502::CMP() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();

        cpu->tmp_buff = (uint16_t)cpu->A - (uint16_t)cpu->data_bus;
        cpu->set_flag(MOS6502::C, cpu->A >= cpu->data_bus);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );
}

void MOS6502::CPX() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)cpu->X - (uint16_t)cpu->data_bus;
        cpu->set_flag(MOS6502::C, cpu->X >= cpu->data_bus);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );
}

void MOS6502::CPY() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
        cpu->tmp_buff = (uint16_t)cpu->Y - (uint16_t)cpu->data_bus;
        cpu->set_flag(MOS6502::C, cpu->Y >= cpu->data_bus);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );
}

void MOS6502::DEC() {

    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
    );

    // TICK(A + 2): Write the value back to effective address, and do the operation on it
    MICROCODE(
        cpu->mem_write();
        cpu->tmp_buff = cpu->data_bus - 1;
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    // TICK(A + 3): Write the new value to effective address
    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::DEX() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->X--;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::DEY() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->Y--;
        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::EOR() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();

        cpu->A = cpu->data_bus ^ cpu->A;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::INC() {

    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
    );

    // TICK(A + 2): Write the value back to effective address, and do the operation on it
    MICROCODE(
        cpu->mem_write();
        cpu->tmp_buff = cpu->data_bus + 1;
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    // TICK(A + 3): Write the new value to effective address
    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::INX() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->X++;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::INY() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->Y++;
        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::JMP() {
    // NOTE(max):   JMP is a particular instruction so need to be treated as an exception
    //              0x4C JMP ABS
    //              0x6C JMP IND
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Fetch low address byte, increment PC
    MICROCODE(
        cpu->address_bus = cpu->PC++;
        cpu->mem_read();
        cpu->tmp_buff = cpu->data_bus & 0x00FF;
    );

    switch (opcode) {
    case 0x4C:  // JMP ABS
        // TICK(3): Copy low address byte to PCL, fetch high address byte to PCH
        MICROCODE(
            cpu->address_bus = cpu->PC;
            cpu->mem_read();
            cpu->PC = (((uint16_t)cpu->data_bus) << 8) | cpu->tmp_buff;
        );
        break;

    case 0x6C:  // JMP IND
        // TICK(3): Fetch pointer address high, increment PC
        MICROCODE(
            cpu->address_bus = cpu->PC++;
            cpu->mem_read();
        );

        // TICK(4): Fetch low address to latch
        MICROCODE(
            cpu->address_bus = (((uint16_t)cpu->data_bus) << 8) | cpu->tmp_buff;
            cpu->mem_read();
            cpu->tmp_buff = cpu->data_bus & 0x00FF;
        );

        // TICK(5): Fetch PCH, copy latch to PCL
        MICROCODE(
            /*
                NOTE(max):  The PCH will always be fetched from the same page
                            than PCL, i.e. page boundary crossing is not handled.
            */
            cpu->address_bus = (((cpu->address_bus & 0x00FF) == 0x00FF)     ?
                                cpu->address_bus & 0xFF00                   :  /* Page boundary hardware bug */
                                cpu->address_bus + 1);

            cpu->mem_read();
            cpu->PC = (((uint16_t)cpu->data_bus) << 8) | cpu->tmp_buff;
        );
        break;

    default:
        log("Unexpected JMP opcode");
        break;
    }
}

void MOS6502::JSR() {
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

void MOS6502::LDA() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
        cpu->A = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::LDX() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();

        cpu->X = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::LDY() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();

        cpu->Y = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::LSR() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
    );

    // TICK(A + 2): Write the value back to effective address, and do the operation on it
    MICROCODE(
        cpu->mem_write();
        cpu->set_flag(MOS6502::C, cpu->data_bus & 0x01);

        cpu->tmp_buff = cpu->data_bus >> 1;

        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, false);
    );

    // TICK(A + 3): Write the new value to effective address
    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::NOP() {
    // TODO(max): fix the nop

    // TICK(A + 1): Read from effective address
    // MICROCODE(
    //     cpu->mem_read();
    //     asm("nop");
    // );
}

void MOS6502::ORA() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();

        cpu->A = cpu->A | cpu->data_bus;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::PHA() {

    // TICK(3): Push register on stack, decrement S
    MICROCODE(
        cpu->data_bus = cpu->A;
        cpu->address_bus = STACK_OFFSET + cpu->S--;
        cpu->mem_write();
    );
}

void MOS6502::PHP() {

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

void MOS6502::ROL() {

    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
    );

    // TICK(A + 2): Write the value back to effective address, and do the operation on it
    MICROCODE(
        cpu->mem_write();
        cpu->tmp_buff = (uint16_t)(cpu->data_bus << 1) | (cpu->read_flag(MOS6502::C) ? 1 : 0);
        cpu->set_flag(MOS6502::C, cpu->tmp_buff & 0xFF00);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    // TICK(A + 3): Write the new value to effective address
    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::ROR() {

    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();
    );

    // TICK(A + 2): Write the value back to effective address, and do the operation on it
    MICROCODE(
        cpu->mem_write();
        cpu->tmp_buff = (uint16_t)((cpu->read_flag(MOS6502::C) ? 1 : 0) << 7) | (cpu->data_bus >> 1);
        cpu->set_flag(MOS6502::C, cpu->data_bus & 0x01);
        cpu->set_flag(MOS6502::Z, (cpu->tmp_buff & 0x00FF) == 0x0000);
        cpu->set_flag(MOS6502::N, cpu->tmp_buff & 0x0080);
    );

    // TICK(A + 3): Write the new value to effective address
    MICROCODE(
        cpu->data_bus = cpu->tmp_buff & 0x00FF;
        cpu->mem_write();
    );
}

void MOS6502::RTI() {

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
void MOS6502::SBC() {
    // TICK(A + 1): Read from effective address
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

void MOS6502::SEC() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->set_flag(MOS6502::C, true);
    );
}

void MOS6502::SED() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->set_flag(D, true);
    );
}

void MOS6502::SEI() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->set_flag(I, true);
    );
}

void MOS6502::STA() {
    // TICK(A + 1): Write register to effective address
    MICROCODE(
        cpu->data_bus = cpu->A;
        cpu->mem_write();
    );
}

void MOS6502::STX() {
    // TICK(A + 1): Write register to effective address
    MICROCODE(
        cpu->data_bus = cpu->X;
        cpu->mem_write();
    );
}

void MOS6502::STY() {
    // TICK(A + 1): Write register to effective address
    MICROCODE(
        cpu->data_bus = cpu->Y;
        cpu->mem_write();
    );
}

void MOS6502::TAX() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->X = cpu->A;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::TAY() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->Y = cpu->A;
        cpu->set_flag(MOS6502::Z, cpu->Y == 0x00);
        cpu->set_flag(MOS6502::N, cpu->Y & 0x80);
    );
}

void MOS6502::TSX() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->X = cpu->S;
        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}

void MOS6502::TXA() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->A = cpu->X;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

void MOS6502::TXS() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->S = cpu->X;
    );
}

void MOS6502::TYA() {
    microcode_q.clear();

    // TICK(1): Fetch opcode, increment PC

    // TICK(2): Read next instruction byte (and throw it away)
    MICROCODE(
        cpu->address_bus = cpu->PC + 1;
        cpu->mem_read();
        cpu->A = cpu->Y;
        cpu->set_flag(MOS6502::Z, cpu->A == 0x00);
        cpu->set_flag(MOS6502::N, cpu->A & 0x80);
    );
}

/********************************************************
 *                  ILLEGAL INST SET                    *
 ********************************************************/

void MOS6502::LAX() {
    // TICK(A + 1): Read from effective address
    MICROCODE(
        cpu->mem_read();

        cpu->A = cpu->data_bus;
        cpu->X = cpu->data_bus;

        cpu->set_flag(MOS6502::Z, cpu->X == 0x00);
        cpu->set_flag(MOS6502::N, cpu->X & 0x80);
    );
}


void MOS6502::SAX() {
    // TICK(A + 1): Write register to effective address
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
