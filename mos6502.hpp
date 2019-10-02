#pragma once
#include <stdint.h>
#include <functional>
#include <vector>
#include "common.hpp"
#include "util.hpp"


#define STACK_POINTER_DEFAULT     0xFD
#define PROCESSOR_STATUS_DEFAULT  0x24
#define INITIAL_ADDRESS           0xFFFC
#define STACK_OFFSET              0x0100


class MOS6502 {
  public:
    explicit MOS6502(mem_access_callback mem_acc_clb, void *usr_data);

    void clock();                     // Clock signal
    void reset();                     // Reset signal
    void irq();                       // Interrupt signal
    void nmi();                       // Non-maskable interrupt signal

    void set_PC(uint16_t address);    // Set the PC to specific memory address. NOTE(max): debug/test

    p_state_t get_status();               // Return the struct containing the current processor status. NOTE(max): debug/test
    void set_log_callback(log_callback);  // Set the callback used for log. Not mandatory

  public:
    /********************************************************
     *                  REGISTERS / FLAGS                   *
     ********************************************************/
    uint8_t A = 0x00;                       // Accumulator
    uint8_t X = 0x00;                       // X Register
    uint8_t Y = 0x00;                       // Y Register
    uint8_t P = PROCESSOR_STATUS_DEFAULT;   // Processor STATUS [N][O][-][B][D][I][Z][C]
    //                                                           |  |  |  |  |  |  |  |
    //                                                           |  |  |  |  |  |  |  +- Carry
    //                                                           |  |  |  |  |  |  +-- Zero
    //                                                           |  |  |  |  |  +--- Interrupt Disable
    //                                                           |  |  |  |  +---- Decimal
    //                                                           |  |  +--+----- No CPU effect, see: the B flag
    //                                                           |  +-------- Overflow
    //                                                           +--------- Negative
    uint8_t S = STACK_POINTER_DEFAULT;      // Stack pointer
    uint16_t PC = 0x0000;                   // Program counter


    /********************************************************
     *                    DATA STRUCTURES                   *
     ********************************************************/
    enum status_flag_t {    // Processor Status Register BITMASKs
        N = (1 << 7),       // N:  NEGATIVE             1 = Neg
        O = (1 << 6),       // O:  OVERFLOW             1 = True
        U = (1 << 5),       // -   UNUSED stay on 1
        B = (1 << 4),       // B:  BRK COMMAND
        D = (1 << 3),       // D:  DECIMAL MODE         1 = True
        I = (1 << 2),       // I:  IRQ DISABLE          1 = Disable
        Z = (1 << 1),       // Z:  ZERO:                1 = Result Zero
        C = (1 << 0)        // C:  CARRY                1 = True
    };

    typedef void (*operation_t)(void);
    typedef void (*addrmode_t)(void);
    typedef void (*micro_op_t)(MOS6502 *self);

    struct instruction_t {  // INSTRUCTION
        const std::string name;
        const operation_t operation;
        const addrmode_t addrmode;
        const unsigned int cycles;
        const unsigned int instruction_bytes;
    };


    /********************************************************
     *                    NEEDED TO WORK                    *
     ********************************************************/
    mem_access_callback mem_access = nullptr;     // Callback used to access the memory
    void *user_data = nullptr;                    // User passed like first argument to the mem_access

    uint8_t opcode;                               // Current opcode
    uint8_t data_bus;                             // Data currently on the bus
    uint16_t address = INITIAL_ADDRESS;           // Current abbsolute address
    uint16_t relative_adderess;                   // Current abbsolute address

    bool accumulator_addressing = false;          // Is True when the current instruction is used with
    //                                               accumulator addressing(the current data is fetched from or written to the A register)

    uint16_t tmp_buff;                            // Temporary 16-bit buffer
    uint16_t hi;
    uint16_t lo;

    // The vector containing the opcode and addressing fuctions and info.
    // The vector is 256 size long and the opcode byte match the correct addressing mode and function
    static const std::vector<instruction_t> opcode_table;

    Queue<micro_op_t, 10> microcode_q;

    /********************************************************
     *                     DEBUG / TEST                     *
     ********************************************************/
    uint16_t PC_executed;             // Address of the last executed opcode      NOTE(max): used only for debug and test. Not need to make the cpu work
    uint8_t arg1;                     // Argument 1 of the last executed opcode   NOTE(max): used only for debug and test. Not need to make the cpu work
    uint8_t arg2;                     // Argument 2 of the last executed opcode   NOTE(max): used only for debug and test. Not need to make the cpu work
    log_callback log_func = nullptr;  // Callback used to log. Can be setted by set_log_callback()

    /********************************************************
     *                    UTIL FUNCTIONS                    *
     ********************************************************/
    void set_flag(const status_flag_t flag, const bool val);
    bool read_flag(const status_flag_t flag);
    void mem_read();
    void mem_write();

  public:
    void log(const std::string &msg);


    /********************************************************
     *                  ADDRESSING MODES                    *
     ********************************************************/
    void ACC();         //                (???)Accumulator addressing:          1-byte instruction on accumulator
    void IMM();         // #$BB           (IMM)Immediate address:               the 2d byte of instruction is the operand
    void ABS();         // $LLHH          (ABS)Abbsolute addressing:            the 2d byte of instruction is the 8 low order bits of the address, 3d is the 8 high order bits (64k total addresses)
    void ZPI();         // $LL            (ZP0)Zero page addressing:            fetch only the 2d byte of the instruction. Assuming the high byte is 0
    void ZPX();         // $LL,X          (ZPX)Indexed zero page addressing X:  the X register is added to the 2d byte. The high byte is 0. No carry is added to high byte, so no page overlapping
    void ZPY();         // $LL,Y          (ZPY)Indexed zero page addressing Y:  Same as in ZPX but with Y register
    void ABX();         // $LLHH,X        (ABX)Indexed abbsolute addressing X:  Adding to X the one absolute address
    void ABY();         // $LLHH,X        (ABY)Indexed abbsolute addressing Y:  Same as in ABX but with Y register
    void IMP();         //                (IMP)Implied addressing:              The address is implicit in the opcode
    void REL();         // $BB            (REL)Relative addressing:             The 2d byte is the branch offset. If the branch is taken, the new address will the the current PC plus the offset.
    //                                                                          The offset is a signed byte, so it can jump a maximum of 127 bytes forward, or 128 bytes backward
    void IIX();         // ($LL,X)        (IZX)Indexed indirect addressing:     The 2d byte is added to the X discarding the carry.
    //                                                                          The result point to the ZERO PAGE address which contains the low order byte of the effective address,
    //                                                                          the next memory location on PAGE ZERO contains the high order byte of the effective address.
    //                                                                     ex:  [LDA ($20,X)] (where is X = $04). X is added so $20 -> $24. Then fetch the $24 -> 0024: 7421.
    //                                                                          The fetched 2171 (little endian) from the memory location 0024 is the actual address to be used to load the content into the register A.
    //                                                                          So id in $2171: 124 then A = 124
    //                                                                          formula: target_address = (X + opcode[1]) & 0xFF
    void IIY();         // ($LL),Y        (IZY)Indirect indexed addressing:     Y is applied to the indirectly fetched address.
    //                                                                     ex:  [LDA ($86),Y] (where in $0086: 28 40). First fetch the address located at $0086, add that address to the Y register to get
    //                                                                          the final address. So the address will be $4028 (little endian) and Y is $10 then the final address is
    //                                                                          $4038 ad A will be loaded with the content of the address $4038
    void IND();         // ($LLHH)        (IND)Absolute indirect:               The 2d byte contain the low byte of address, the 3d byte contain the high byte of the address.
    //                                                                          The loaded address contain the low byte fo the final addrss and the followed the high byte of the final address

    /********************************************************
     *                   INSTRUCTION SET                    *
     ********************************************************/
    void ADC();         // Add memory to A with Carry
    void AND();         // "AND" memory with accumulator
    void ASL();         // Shift LEFT 1 bit (memory or A)

    void BCC();         // Branch on Carry Clear
    void BCS();         // Branch on Carry Set
    void BEQ();         // Branch on Result Zero
    void BIT();         // Test Bits in Memory with A
    void BMI();         // Branch on Result Minus
    void BNE();         // Branch on Result not Zero
    void BPL();         // Branch on Result Plus
    void BRK();         // Force Break
    void BVC();         // Branch on Overflow Clear
    void BVS();         // Branch on Overflow Set

    void CLC();         // Clear Carry Flag
    void CLD();         // Clear Decimal Mode
    void CLI();         // Clear Interrupt Disable Bit
    void CLV();         // Clear Overflow Flag
    void CMP();         // Compare Memory and accumulator
    void CPX();         // Compare Memory and X
    void CPY();         // Compare Memory and Y

    void DEC();         // Decrement Memory by 1
    void DEX();         // Decrement Index X by 1
    void DEY();         // Decrement Index Y by 1

    void EOR();         // "Exclusive-OR" Memory with A

    void INC();         // Increment Memory by 1
    void INX();         // Increment Index X by 1
    void INY();         // Increment Index  by 1

    void JMP();         // Jump to New Location
    void JSR();         // Jump to New Location Saving Return Address

    void LDA();         // Load A with the Memory
    void LDX();         // Load X with the Memory
    void LDY();         // Load Y with the Memory
    void LSR();         // Shift 1 bit RIGHT (Memory or A)

    void NOP();         // No Operation

    void ORA();         // "OR" Memory with the A

    void PHA();         // Push A on Stack
    void PHP();         // Push P on Stack
    void PLA();         // Pull A from Stack
    void PLP();         // Pull P from Stack

    void ROL();         // Rotate 1 bit LEFT (Memory or Accumulator)
    void ROR();         // Rotate 1 bit RIGHT (Memory or Accumulator)
    void RTI();         // Return from Interrupt
    void RTS();         // Return from Subroutine

    void SBC();         // Subtract Memory from Accumulator with Borrow
    void SEC();         // Set Carry Flag
    void SED();         // Set Decimal Mode
    void SEI();         // Set Interrupt Disable Status
    void STA();         // Store A in Memory
    void STX();         // Store Index X in Memory
    void STY();         // Store Index Y in Memory

    void TAX();         // Transfer A to Index X
    void TAY();         // Transfer A to Index Y
    void TSX();         // Transfer S (Stack Pointer) to Index X
    void TXA();         // Transfer Index X to A
    void TXS();         // Transfer Index X to Stack Register
    void TYA();         // Transfer Index Y to A

    /********************************************************
     *                  ILLEGAL INST SET                    *
     ********************************************************/
    void LAX();         // Loads a value from an absolute address in memory and stores it in A and X at the same time
    void SAX();         // Stores the bitwise AND of A and X. As with STA and STX, no flags are affected.
    void DCP();         // Equivalent to DEC value then CMP value, except supporting more addressing modes. LDA #$FF followed by DCP can be used to check if the decrement underflows, which is useful for multi-byte decrements.
    void ISB(); // ISC  // Equivalent to INC value then SBC value, except supporting more addressing modes.
    void SLO();         // Equivalent to ASL value then ORA value, except supporting more addressing modes. LDA #0 followed by SLO is an efficient way to shift a variable while also loading it in A.
    void RLA();         // Equivalent to ROL value then AND value, except supporting more addressing modes. LDA #$FF followed by RLA is an efficient way to rotate a variable while also loading it in A.
    void SRE();         // Equivalent to LSR value then EOR value, except supporting more addressing modes. LDA #0 followed by SRE is an efficient way to shift a variable while also loading it in A.
    void RRA();         // Equivalent to ROR value then ADC value, except supporting more addressing modes. Essentially this computes A + value / 2, where value is 9-bit and the division is rounded up.

    void XXX();         // Illegal instruction
};