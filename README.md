# ESTC OS-les build system

Build system for ESTC OS-less tasks

## Prerequisites
* [arm-none-eabi](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) compiler
* [stlink](https://github.com/texane/stlink) utility

## System reference
* [common_defs.mk](common_defs.mk) -- file with many useful rules. Every makefile that builds something should include it and define `BUILD_ROOT` variable -- path to this file.
* [Makefile](Makefile) -- "main hub" for a project. Here you can add you workshop directories and call `make` on them. Take a look at the example `lab1` target.
* [common](common) -- directory with various common stuff like startup files and interrupt tables. It's compiled once as a static library `libstmcommon.a` and linked with every binary produced with the build system.
* [stm_spl](stm_spl) -- [Standard Peripherials Library](https://www.st.com/en/embedded-software/stsw-stm32065.html) form ST. It makes development much easier but with an overhead.
* [projects](projects) -- directory with workshops. Every workshop should be in a separate directory inside and should define its own makefile rules. Take a look at the [led_test](projects/led_test) as an example.

## Various notes
* You can put worhskops git repository to [projects](projects) directory.
* It's possible to use helper commands from [common_defs.mk](common_defs.mk) to link your binaries. Example:

``` make
main: main.o
    @echo "Linking ELF"
    $(LINK_ELF)
    @echo "Making image"
    $(MAKE_IMAGE)
```

## Debugging 
If you have `arm-none-eabi-gdb` binary installed, then you can skip right to step 2 of this instruction. Otherwise, stat from the beginning
1. Install gdb for ARM. In ubuntu:
   ```
   #> sudo apt install gdb-mutiarch
   ```
2. Connect a board and use `st-util` to start a GB server on it:
   ```
   ?> st-util -m -p 4242
   ```
3. Navigate to your project directory and launch gdb. For ubuntu: 
   ``` 
   ?> gdb-multiarch main.elf
   ```
4. Connect to a gdb server on board and load your binary:
   ```
   gdb> target extended-remote :4242
   gdb> load main.elf
   gdb> continue
   ```
   Tou can put those lines (omitting `gdb>` prefix) to the `~/.gbinit` file.
5. Press a reset on the board
6. Debug normally. GDB reference: [GDB refcard](http://users.ece.utexas.edu/~adnan/gdb-refcard.pdf).
