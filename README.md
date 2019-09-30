# Another MOS 6502 Emulator
This is my implementation of the MOS6502 processor emulator

# Project overview
* To **compile** all project just type in the root folder `make all`. 
* To **run tests** `make test`
* To run the **console cpu** controller `./emu name_of_the_binary`, like `./emu resources/program.bin`


TODO

# Credits
* To [Javidx9](http://www.onelonecoder.com/index.html) for the inspiration

# Libs
* **[doctest](https://github.com/onqtam/doctest)**: It's one header C++ fast test framework 

* **[nestest.nes](http://nickmass.com/images/nestest.nes)**: Used with the Nintendulator [logs](http://www.qmtpro.com/~nes/misc/nestest.log) to test the processor. The test consist in compare the log from the my mos6502 after each instruction with the log from the [Nintendulator](https://wiki.nesdev.com/w/index.php/Nintendulator). The **nestest.nes** can be used with and without graphical mode. To perform all test without ppu (like my test) you need to load the cartridge and set the program counter to address 0xC000, like described in this [doc](http://www.qmtpro.com/~nes/misc/nestest.txt). NOTE(max): this will test also the illegal opcodes. 

# Resources
* [Javidx9](http://www.onelonecoder.com/index.html)'s NES Emulator [videos](https://www.youtube.com/watch?v=8XmxKPJDGU0&t=1s) and [repo](https://github.com/OneLoneCoder/olcNES). Used like reference for:
    * The opcode table
    * Nes cartridge loader and memory management used for execute the       nestest.nes cartridge for test the processor
    * Reference for most of the addressing mode and operations implementation

* Gianluca Ghettini [website](https://www.gianlucaghettini.net/mos-6502-cpu-emulator-in-c/) for some information

* [visual6502.org](visual6502.org/) for documentation, technical and general purpose information. Also the **FANTASTIC** [simulator](http://www.visual6502.org/JSSim/index.html) for general debug and behaviour comparison. This is the BEST place if you need information about mos6502

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

* [Datasheet](https://www.mdawson.net/vic20chrome/cpu/mos_6500_mpu_preliminary_may_1976.pdf)

* [Rockwell Datasheet](http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf) with opcode matrix


# License

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