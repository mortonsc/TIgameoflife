# TIgameoflife

![screenshot](/screenshots/gameplay.gif)

A simple implementation in C of [John Conway's Game of Life](https://en.wikipedia.org/wiki/Conway's_Game_of_Life) for TI-83+ series calculators.

## Supported Calculators
This program is tested on a TI-84+.
It should be compatible with the TI-83+, TI-83+SE, TI-84+, and TI-84+SE.
However, it will run noticeably slower on the TI-83+, which has a slower CPU.

## Compilation
The source code for this program is hosted on [Github](https://github.com/mortonsc/TIgameoflife "mortonsc/TIgameoflife").
To compile the program, you will need the following software:
* [SDCC](http://sdcc.sourceforge.net/ "SDCC") (Small Device C Compiler): compiler/assembler/linker
* [hex2bin](http://hex2bin.sourceforge.net/ "hex2bin"): converts output from SDCC to binary file
* [binpac8x.py](http://www.ticalc.org/archives/files/fileinfo/429/42915.html "binpac8x"): packages binary file into executable for the calculator

The makefile assumes all these programs are in your system path.
On Linux, it may also be necessary to change the permissions of binpac8x.py so you can execute it: `chmod +x binpac8x.py`.

### Linux
To download and compile the source: open a terminal and navigate to the directory you want to download the source to.
Then enter the commands:

    git clone --recursive https://github.com/mortonsc/TIgameoflife.git
    make lib
    make
That's it! The executable file is `game_of_life.8xp`.

### Windows
All the required software is available for Windows, so it should be possible to compile, but I haven't tried.

## Installation
Currently the only official way to obtain an executable is to follow the compilation process above.
Once you've done that, open `game_of_life.8xp` with your favorite emulator, or, if you're particularly trusting,
upload it to your calculator with your favorite linking software.
As far as I know the program is safe,
but bear in mind that running assembly programs on the calculator is inherently dangerous.

## How to Play
The compiled program is called CONWAY on the calculator. To run, use the `Asm(` command (found in the catalog):

    Asm(prgmCONWAY
The program does not offer any way to edit the screen; the contents of your graph screen at the time you run the program
are used as the initial state. If you want to quickly create an interesting screen, you can use the `Draw->Text(` command
to print text to the screen.

When the program starts, it is paused. There are 5 possible actions:
* `Clear`: quits the program
* `-`: advances the game one step
* `+`: starts the game running
* `Sto`+Num: Stores the contents of the screen to picture variable Num.
Num is any of the numerical keys.  (as if you had entered `StorePic` Num)
* `Ln`+Num: Loads the contents of picture variable Num to the screen.
(as if you had entered `RecallPic` Num)

While the game is running, pressing `Enter` will pause it again. You may have to hold the button for a second
before the game pauses.

## Planned Features
This is a list of features I plan to add:
* About/help screen
* Wrapping around edges
* Generation number tracking

## Contact
You can contact me via email; my address is visible on my Github profile.
Bug reports should include enough information for me to replicate the error.
