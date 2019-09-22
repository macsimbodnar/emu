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

  private:
    // Addressing modes
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


    void set_flag(const status_flag_t flag, const bool val);
    bool read_flag(const status_flag_t flag);

  public:
    MOS6502(Bus *b);

    void clock();       // Clock signal
    void reset();       // Reset signal
    void irq();         // Interrupt signal
    void nmi();         // Non-maskable interrupt signal
};