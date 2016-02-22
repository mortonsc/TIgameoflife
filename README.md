# TIgameoflife

![screenshot](/screenshots/gameplay.gif)

A simple implementation in C of [John Conway's Game of Life](https://en.wikipedia.org/wiki/Conway's_Game_of_Life) for TI-83+ series calculators.
If you want to write your own C programs on a calculator, this project provides a helpful framework.

## Supported Calculators
This program is tested on a TI-84+.
The 83+, 83+SE, 84+, and 84+SE are all compatible on an assembly level, so it should work on any of them.
It will run significantly slower on the standard TI-83+, which has a slower processor.

## Compilation
The source code for this program is hosted on [github](https://github.com/mortonsc/TIgameoflife "mortonsc/TIgameoflife").
To compile the program, you will need the following software:
* [SDCC](http://sdcc.sourceforge.net/ "SDCC") (Small Device C Compiler): compiler/assembler/linker
* [hex2bin](http://hex2bin.sourceforge.net/ "hex2bin"): converts output from SDCC to binary file
* [binpac8x.py](http://www.ticalc.org/archives/files/fileinfo/429/42915.html "binpac8x"): packages binary file into executable for the calculator

The included Makefile assumes all these programs are in your system path.
On Linux, it may also be necessary to change the permissions of binpac8x.py so you can execute it: `chmod +x binpac8x.py`.

### Linux
To compile:

    git clone https://github.com/mortonsc/TIgameoflife.git
    make
That's it!

### Windows
All the required software is available for Windows, so it should be possible to compile, but I haven't tried.

## Installation
Currently the only way to obtain an executable is to follow the compilation process above.
Once you've done that, upload `game_of_life.8xp` to your favorite emulator, or, if you're particularly trusting,
upload it to your calculator with your favorite linking software.
As far as I know the program is stable,
but bear in mind that running assembly programs on the calculator is inherently dangerous.

## How to Play
The compiled program is called CONWAY on the calculator. To run, use the `Asm(` command (found in the catalog):

    Asm(prgmCONWAY
The program does not offer any way to edit the screen; the contents of your graph screen at the time you run the program
are used as the initial state. If you want to quickly create an interesting screen, you can use the `Draw->Text(` command
to print text to the screen.

When the program starts, it is paused. There are 3 possible actions:
* `Clear`: quits the program
* `-`: advances the game one step
* `+`: starts the game running

While the game is running, pressing `Enter` will pause it again. You will probably have to hold the button for a second
before the game pauses.

## Planned Features
This is a list of features I plan to add:
* About/help screen
* Generation number tracking
* Ability to load/save to the calc's picture variables

## Included Files
If you want to write your own C program for the calculator,
I recommend you avoid the pain I went through and use this project as a starting point.
The `inc/` directory includes:
* [tios_crt0.s](inc/tios_crt0.s): The crt0 (C RunTime 0) file appears at the very beginning of the compiled assembly program.
It needs to be correct for the compiled program to be runnable on the calculator.
* [ti84plus.h](inc/ti84plus.h): A header file containing C wrappers for various system calls and defines for keycodes and special characters. This is one is perpetually in development as I add support for more features.
If there's something you need, contact me and I'll see if I can't add it in.
* [fastcopy.h](inc/fastcopy.h): A C wrapper for the [fastcopy](http://wikiti.brandonw.net/index.php?title=Z80_Routines:Graphic:Fastcopy) assembly routine,
which copies the contents of a buffer to the LCD.

In addition, the [makefile](Makefile) has all the correct flags for compiling/assembling with SDCC,
and can be easily modified to work with your source files.

## Contact
You can contact me via email; my address is visible on my Github profile.
Bug reports should include enough information for me to replicate the error.
