# Another MOS 6502 Emulator

This is my implementation of the MOS6502 processor emulator

## Project overview

* `libs`:
  * `doctest.h`: C++ test framework

* `resources`:
  * `hex_to_bin.py`: Python3 script that take in input a hex formatted file and   translate it into a binary file ignoring special characters. To run is type:   `python3 hex_to_bin.py my_hex_file.hex` and teh output will be in   `my_hex_file.bin`. You can load the binary directly int the emu.
  * `nestest.log`: Nintendulator logs of the execution of `nestest.nes`. It is used   to test my processor.
  * `nestest.nes`: NES Cartridge with the test almost all of the mos6502   functionalities. Also include tests of the unofficial opcodes
  * `program.hex`: this is the example program to run on the emu. The source is:

      ```assembly
      LDX #10
      STX $0000
      LDX #3
      STX $0001
      LDY $0000
      LDA #0
      CLC
      loop
      ADC $0001
      DEY
      BNE loop
      STA $0002
      NOP
      NOP
      NOP
      ```

  * `program.bin`: This is the binary version of the `program.hex`

* `common`: Contains some common data types that a potential user of MOS6502 class will need

* `console`: This is a console program called **EMU** that allow to use the mos6502 emulator and perform debug step by step. To run it with a sample program just first build the project and then run `./emu resources/program.bin`

* `mos6502`: Contains the implementation of the mos6502 emulator

* `opcode`: Contains the opcode table that map the instruction code to the addressing mode, operation, size of operation, num of clocks needed ad the mnemonic of the operation. The mnemonic prefixed by the `*` are **unofficial** operations.

* `test`: This is the file used to test the emulator. It loads the NES Cartridge `nestest.nes`

## How to compile

* To **compile the whole project** just type in the root folder `make all`.
* To **compile only the test** run `make build_test`

## How to run

* To **run tests** `make test`
* To run the **console cpu** controller nicely named **EMU** `./emu name_of_the_binary`, like `./emu resources/program.bin`

## **EMU** user instruction

First of all you need to set you terminal size at least to 95x30.

In the left corner you will see the current memory page printed in hex.

In the MOS6502 a memory page is a chunk of memory of the size of 256 bytes and start at the address 0xHH00 and continue up to 0xHHFF.

The First page is called **Zero Page** (from 0x0000 to 0x00FF) and is special. The processor have shortcut to access this page and have his own addressing mode. The Zero Page is used often like registers.

The second page (from 0x0100 t 0x01FF) is used for the stack. By reference the stack pointer is set to 0x01FD and decreases during the use.

The `*` near the byte in the memory corner means the Program counter currently point here. The `$` is the current address selected on the data bus (this is the address where the current data is read or written). The `#` means that `*` and `$` point to the same memory location.

In the right corner are printed all processor states like the register A, the indexes X and Y, the Stack Pointer, the Program Counter and some more information not present in the real processor but used by me for the inner logic.

Under below you can find a line with the information about the current instruction.

Then we have 5 lines used for print logs.

### **EMU** commands

The **EMU** is case insensitive for now

* `b`: show **previous** memory page
* `c`: perform one clock cycle
* `l`: print some test log (currently used for debug the log functionality)
* `n`: show **next** memory page
* `q`: quit

## TODOs

### Now

* Fix the interrupts timing
* Replace tmp_buff uint16_t with tmp_h and tmp_l uint8_t
* Rewrite all casts from C style to static_cast C++ like
* Check the timing of the composed illegal opcodes
* Implement decimal mode
* Test the interrupts
* Implement clock
* Test speed on ARMs processors (like raspberry or BeagleBone or TechNexion)

### In the future

* In **EMU** set the current according on the status of the processor. This feature should be turned on and off by the user
* Better interface for the **EMU**
* More user friendly and complex commands for the **EMU**
* Introduce some other tests beside the `nestest.nes`
* Will be nice to find more logs of `nestest.nes` and test against it too
* Integrate into **EMU** the program loader directly from hex formatted file
* Maybe also integrate (or maybe write) some assembler into **EMU**

## Libs

* **[doctest](https://github.com/onqtam/doctest)**: It's one header C++ fast test framework

* **[nestest.nes](http://nickmass.com/images/nestest.nes)**: Used with the Nintendulator [logs](http://www.qmtpro.com/~nes/misc/nestest.log) to test the processor. The test consist in compare the log from the my mos6502 after each instruction with the log from the [Nintendulator](https://wiki.nesdev.com/w/index.php/Nintendulator). The **nestest.nes** can be used with and without graphical mode. To perform all test without ppu (like my test) you need to load the cartridge and set the program counter to address 0xC000, like described in this [doc](http://www.qmtpro.com/~nes/misc/nestest.txt). NOTE(max): this will test also the illegal opcodes.

## Resources

* [Javidx9](http://www.onelonecoder.com/index.html)'s NES Emulator [videos](https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1s) and [repo](https://github.com/OneLoneCoder/olcNES). Used like reference for:
  * The opcode table
  * Nes cartridge loader and memory management used for execute the         nestest.nes cartridge for test the processor
  * Reference for most of the addressing mode and operations implementation

* Gianluca Ghettini [website](https://www.gianlucaghettini.net/mos-6502-cpu-emulator-in-c/) for some information

* [visual6502.org](visual6502.org/) for documentation, technical and general purpose information. Also the **FANTASTIC** [simulator](http://www.visual6502.org/JSSim/index.html) for general debug and behavior comparison. This is the BEST place if you need information about mos6502

* Obviously [wikipedia](https://en.wikipedia.org/wiki/MOS_Technology_6502)

* Nice [speech](https://www.youtube.com/watch?v=fWqBmmPQP40&list=FLYst_qeLEOiWw8iSiueDJRw&index=3&t=1846s) about the mos6502 reverse engineering

* Some details about the addressing modes at the [emulator101](http://www.emulator101.com/6502-addressing-modes.html)

* Online mos6502 [assembler](https://www.masswerk.at/6502/assembler.html)

* 6502 instruction set [information](https://www.masswerk.at/6502/6502_instruction_set.html)

* Test reference at [nesdev,com](https://wiki.nesdev.com/w/index.php/Emulator_tests)

* [6502.org](http://www.6502.org/tutorials/6502opcodes.html) for detailed information about addressing modes and operations

* [Easy6502](http://skilldrick.github.io/easy6502/) for debug

* Information at [pagetable.com](https://www.pagetable.com/?p=410) about BRK, IRQ, NMI and RESET

* Nice [emulator/debugger](http://e-tradition.net/bytes/6502/) used for debugging

* Unofficial opcodes [information](https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes)

* More unofficial opcodes [information](https://wiki.nesdev.com/w/index.php/Programming_with_unofficial_opcodes)

* Detailed [document](http://atarihq.com/danb/files/64doc.txt) about the processor behavior

* Synertek programming [manual](http://archive.6502.org/datasheets/synertek_programming_manual.pdf)

* Synertek hardware [manual](http://archive.6502.org/datasheets/synertek_hardware_manual.pdf)

* [Datasheet](https://www.mdawson.net/vic20chrome/cpu/mos_6500_mpu_preliminary_may_1976.pdf)

* [Rockwell Datasheet](http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf) with opcode matrix

* [Document](http://www.zimmers.net/anonftp/pub/cbm/documents/chipdata/64doc) used for **timing** in the cycle-precise version. It is about 6510 but the instructions set should be the same as the 6502

* Nice [Instruction timing](http://nparker.llx.com/a2/opcodes.html#chart) description and more info

* Detailed [Interrupts description](https://en.wikipedia.org/wiki/Interrupts_in_65xx_processors) of 65XX processors

## License

Copyright (c) 2019 Maksym Bodnar aka Mazerfaker

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
