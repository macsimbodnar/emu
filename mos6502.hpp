/**
 *  address size:       16  bit
 *  data size:          8   bit
 *
 *  OpCodes:            XXXX            XXXX
 *                      Addressing      Instruction
 *                      Mode
 */
#pragma once
#include <stdint.h>
#include "bus.hpp"


class MOS6502 {
  private:
    uint8_t A;          // Accumulator
    uint8_t X;          // X Register
    uint8_t Y;          // Y Register

    uint16_t PC;        // Program counter
    uint8_t S;          // Stack pointer            NOTE(max): [1][---- ----]

    enum status_flag_t {
        N = (1 << 7),   // N:  NEGATIVE             1 = Neg
        O = (1 << 6),   // O:  OVERFLOW             1 = True
        //                 -
        B = (1 << 4),   // B:  BRK COMMAND
        D = (1 << 3),   // D:  DECIMAL MODE         1 = True
        I = (1 << 2),   // I:  IRQ DISABLE          1 = Disable
        Z = (1 << 1),   // Z:  ZERO:                1 = Result Zero
        C = (1 << 0)    // C:  CARRY                1 = True
    };

    uint8_t P;          // Processor status reg     NOTE(max): [N][V][-][B][D][I][Z][C]

    Bus *bus;

    void set_flag(const status_flag_t flag, const bool val);
    bool read_flag(const status_flag_t flag);

  public:
    MOS6502(Bus *b);

    void clock();       // Clock signal
    void reset();       // Reset signal
    void irq();         // Interrupt signal
    void nmi();         // Non-maskable interrupt signal


  private:
    /********************************************************
     *                  ADDRESSING MODES                    *
     ********************************************************/
    bool ACC();         // Accumulator addressing:          1-byte instruction on accumulator
    bool IMM();         // Immediate address:               the 2d byte of instruction is the operand
    bool ABS();         // Abbsolute addressing:            the 2d byte of instruction is the 8 low order bits of the address, 3d is the 8 high order bits (64k total addresses)
    bool ZPI();         // Zero page addressing:            fetch only the 2d byte of the instruction. Assuming the high byte is 0
    bool ZPX();         // Indexed zero page addressing X:  the X register is added to the 2d byte. The high byte is 0. No carry is added to high byte, so no page overlapping
    bool ZPY();         // Indexed zero page addressing Y:  Same as in ZPX but with Y register
    bool ABX();         // Indexed abbsolute addressing X:  Adding the X the one absolute address
    bool ABY();         // Indexed abbsolute addressing Y:  Same as in ABX but with Y register
    bool IMP();         // Implied addressing:              The address is implicit in the opcode
    bool REL();         // Relative addressing:             Only with brach instruction. Is the destination for the conditional branch. The 2d byte is added to the PC (range from -128 to 127)
    bool IIX();         // Indexed indirect addressing:     The 2d byte is added to the X discarding the carry.
    //                                  The result point to the ZERO PAGE address which contains the low order byte of the effective address,
    //                                  the next memory location on PAGE ZERO contains the high order byte of the effective address.
    //                              ex: [LDA ($20,X)] (where is X = $04). X is added so $20 -> $24. Then fetch the $24 -> 0024: 7421.
    //                                  The fetched 2171 (little endian) from the memory location 0024 is the actual address to be used to load the content into the register A.
    //                                  So id in $2171: 124 then A = 124
    //                                  formula: target_address = (X + opcode[1]) & 0xFF
    bool IIY();         // Indirect indexed addressing:     Y is applied to the indirectly fetched address.
    //                              ex: [LDA ($86),Y] (where in $0086: 28 40). First fetch the address located at $0086, add that address to the Y register to get
    //                                  the final address. So the address will be $4028 (little endian) and Y is $10 then the final address is
    //                                  $4038 ad A will be loaded with the content of the address $4038
    bool IND();         // Abolute indirect:                The 2d byte contain the low byte of address, the 3d byte contain the high byte of the address.
    //                                  The loaded address contain the low byte fo the final addrss and the followed the high byte of the final address

    /********************************************************
     *                   INSTRUCTION SET                    *
     ********************************************************/
    bool ADC();         // Add memory to A with Carry
    bool AND();         // "AND" memory with accumulator
    bool ASL();         // Shift LEFT 1 bit (memory or A)

    bool BCC();         // Branch on Carry Clear
    bool BCS();         // Branch on Carry Set
    bool BEQ();         // Branch on Result Zero
    bool BIT();         // Test Bits in Memory with A
    bool BMI();         // Branch on Result Minus
    bool BNE();         // Branch on Result not Zero
    bool BPL();         // Branch on Result Plus
    bool BRK();         // Force Break
    bool BVC();         // Branch on Overflow Clear
    bool BVS();         // Branch on Overflow Set

    bool CLC();         // Clear Carry Flag
    bool CLD();         // Clear Decimal Mode
    bool CLI();         // Clear Interrupt Disable Bit
    bool CLV();         // Clear Overflow Flag
    bool CMP();         // Compare Memory and accumulator
    bool CPX();         // Compare Memory and X
    bool CPY();         // Compare Memory and Y

    bool DEC();         // Decrement Memory by 1
    bool DEX();         // Decrement Index X by 1
    bool DEY();         // Decrement Index Y by 1

    bool EOR();         // "Exclusive-OR" Memory with A

    bool INC();         // Increment Memory by 1
    bool INX();         // Increment Index X by 1
    bool INY();         // Increment Index  by 1

    bool JMP();         // Jump to New Location
    bool JSR();         // Jump to New Location Saving Return Address

    bool LDA();         // Load A with the Memory
    bool LDX();         // Load X with the Memory
    bool LDY();         // Load Y with the Memory
    bool LSR();         // Shift 1 bit RIGHT (Memory or A)

    bool NOP();         // No Operation

    bool ORA();         // "OR" Memory with the A

    bool PHA();         // Push A on Stack
    bool PHP();         // Push P on Stack
    bool PLA();         // Pull A from Stack
    bool PLP();         // Pull P from Stack

    bool ROL();         // Rotate 1 bit LEFT (Memory or Accumulator)
    bool ROR();         // Rotate 1 bit RIGHT (Memory or Accumulator)
    bool RTI();         // Return from Interrupt
    bool RTS();         // Return from Subroutine

    bool SBC();         // Subtract Memory from Accumulator with Borrow
    bool SEC();         // Set Carry Flag
    bool SED();         // Set Decimal Mode
    bool SEI();         // Set Interrupt Disable Status
    bool STA();         // Store A in Memory
    bool STX();         // Store Index X in Memory
    bool STY();         // Store Index Y in Memory

    bool TAX();         // Transfer A to Index X
    bool TAY();         // Transfer A to Index Y
    bool TSX();         // Transfer S (Stack Pointer) to Index X
    bool TXA();         // Transfer Index X to A
    bool TXS();         // Transfer Index X to Stack Register
    bool TYA();         // Transfer Index Y to A

};