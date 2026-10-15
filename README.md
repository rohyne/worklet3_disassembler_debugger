# Worklet 3: Disassembler & Step Debugger

## What's this?

This is our part of Mini Project 15. The goal is to make it easier to see what a program is actually doing under the hood.

We take machine instructions, turn them back into readable assembly, and let you step through a program one instruction at a time to watch how it behaves.

## What we're aiming for

- Build a disassembler that shows machine instructions as readable assembly
- Add a step debugger so you can watch a program run bit by bit
- Show instruction-level info so the program flow actually makes sense
- Dig into how instructions execute and how control flow works
- Plug it all into the rest of the project

## Planned features

- **Instruction disassembly:** machine code in, readable assembly out
- **Step-by-step debugging:** run one instruction at a time and see what happens
- **Instruction display:** addresses, opcodes, and the decoded instruction side by side
- **Execution tracking:** always know which instruction you're on and where the program is heading
- **Error handling:** bad input or unsupported instructions shouldn't crash anything

## Tech stack

- **Language:** C/C++
- **Editor/IDE:** Visual Studio Code or Visual Studio
- **Version control:** Git and GitHub
- **Target platform:** Windows

## Project structure

```text
Worklet-3/
├── README.md
├── src/
│   └── (implementation files)
├── include/
│   └── (header files)
├── tests/
│   └── (test files)
└── docs/
    └── (documentation)
```

This will probably change as we build things out.

## The plan

1. Get familiar with the overall Mini Project 15 architecture and nail down our interfaces
2. Decide which instruction format and architecture we're supporting
3. Design the disassembly module
4. Build the instruction decoding and formatted assembly output
5. Design and build the step debugger
6. Test everything with sample programs and instruction sequences
7. Integrate with the rest of the project
8. Write up the implementation, limitations, and test results

## What we're hoping to end up with

A working disassembler and step debugger that gives readable instruction-level info and lets you step through a program, all integrated with the rest of Mini Project 15.