# CHIP-8 Emulator (C++)

A **minimal, from-scratch CHIP-8 emulator** implemented in **C++**, focused on correctness, clarity, and opcode-level fidelity. This project emulates the original CHIP-8 virtual machine and runs classic ROMs such as **Pong** and **Tetris**.

---

## Overview

CHIP-8 is an interpreted programming language developed in the 1970s for simple 8-bit systems. This emulator recreates the complete CHIP-8 execution environment, including:

* **4 KB memory model**
* **16 general-purpose 8-bit registers (V0–VF)**
* **Index register (I)**
* **Program counter (PC)**
* **Stack and stack pointer**
* **Delay and sound timers**
* **64 × 32 monochrome display**
* **16-key hexadecimal keypad**
* **Full opcode decoding and execution**

The implementation is written entirely in **C++**, without relying on external emulator frameworks.

---

## Project Structure

```
.
├── chip8              # Compiled executable
├── chip8.cpp          # Emulator source code
├── LICENSE            # License file
├── README.md          # Project documentation
└── ROM                # CHIP-8 ROMs
    ├── Pong.ch8
    ├── test_opcode.ch8
    └── tetris.ch8
```

---

## Build Instructions

### Requirements

* macOS or Linux
* `clang++` or `g++`
* SDL2 (required only if graphics/audio output is enabled)

### Compilation

```bash
clang++ chip8.cpp -o chip8
```

With SDL2:

```bash
clang++ chip8.cpp -o chip8 \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  -lSDL2
```

---

## Running the Emulator

Run the emulator by passing a CHIP-8 ROM as a command-line argument:

```bash
./chip8 ROM/Pong.ch8
```

Additional examples:

```bash
./chip8 ROM/tetris.ch8
./chip8 ROM/test_opcode.ch8
```

---

## Features

* Complete CHIP-8 instruction set implementation
* Accurate opcode fetch–decode–execute cycle
* ROM loading starting at memory address `0x200`
* Built-in fontset loading at standard locations
* Deterministic execution loop
* Modular opcode handler design
* Support for test ROM–based validation

---

## Included ROMs

* **Pong.ch8** — Classic Pong implementation
* **tetris.ch8** — Fully playable Tetris
* **test_opcode.ch8** — Opcode verification ROM
