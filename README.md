# c-riscv-text-normalizer-counter

RISC-V assembly implementations of text normalization and word counting, benchmarked against a C implementation under QEMU.

## Description

The program processes a string in place and does two things:

1. **Normalizes** the text by converting uppercase letters (`A-Z`) to lowercase.
2. **Counts words**, where a word is a run of non-whitespace characters.

Three implementations of the same function are included and timed against each other:

| Version | File | Function |
|---|---|---|
| C | `main.c` | `process_text_c` |
| Unoptimized assembly | `unoptimized.S` | `process_text_asm` |
| Optimized assembly | `optimized.S` | `process_text_asm_opt` |

`main.c` runs each version on the same input, compares the outputs against the C result, and reports the execution times.

## Project Structure

```
.
├── main.c           # C implementation, test driver, and timing code
├── unoptimized.S    # RISC-V assembly, direct translation of the C logic
├── optimized.S      # RISC-V assembly, branch-reduced version
└── README.md
```

## Requirements

- Linux, or Windows with WSL (Ubuntu)
- RISC-V GNU cross-compiler (`gcc-riscv64-linux-gnu`)
- QEMU user-mode emulator for RISC-V (`qemu-user`)

## Setup

Install the required packages on Ubuntu or WSL:

```bash
sudo apt update
sudo apt install gcc-riscv64-linux-gnu qemu-user
```

Check that both tools are available:

```bash
riscv64-linux-gnu-gcc --version
qemu-riscv64 --version
```

## Build and Run

Compile all source files into one statically linked RISC-V executable:

```bash
riscv64-linux-gnu-gcc -O2 -static main.c unoptimized.S optimized.S -o benchmark
```

Run it with QEMU:

```bash
qemu-riscv64 benchmark
```

Or do both in one line:

```bash
riscv64-linux-gnu-gcc -O2 -static main.c unoptimized.S optimized.S -o benchmark && qemu-riscv64 benchmark
```

## Configuration

The number of timed iterations is set at the top of `main.c`:

```c
#define ITERATIONS 100000
```

Each iteration copies the original string into a buffer and then processes it, so the run is repeated on the unedited original text with uppercase input every time.

## Output

For each version, the program prints the normalized text, the word count, and the execution time. Results are also checked automatically. The program prints whether the normalized text and word count of each assembly version match the C version.

```
Original Text:
...

strcpy Baseline (100000 iterations): ... ms

Normalize and Word Count (C):
...
Word Count (C): ...
C Execution Time (100000 iterations): ... ms
C Net Time (baseline subtracted): ... ms

(same format for ASM and ASM Optimized)

No mismatch found! Normalization successful.
No mismatch found! Word Counts are equal.
...
```

## How Timing Works

- Time is measured with `clock_gettime(CLOCK_MONOTONIC, ...)`.
- Every iteration includes a `strcpy` to reset the buffer, and this cost is the same for all versions.
- The `strcpy` cost is measured separately as a baseline, and the **Net Time** is the execution time minus this baseline. Net Time reflects the processing function alone and is the better number for comparing versions.

## How the Implementations Differ

**C (`process_text_c`):** loops over the string, tracks whether the current position is inside a word, increments the word count when a new word starts, and converts uppercase letters to lowercase.

**Unoptimized assembly (`process_text_asm`):** a direct translation of the C logic. Each character goes through several compare-and-branch checks for whitespace and for the `A-Z` range, plus a branch at the top and a jump at the bottom of the loop.

**Optimized assembly (`process_text_asm_opt`):** the same logic with most branches replaced by arithmetic:
- Whitespace is detected with a single compare (`c < 33`) instead of several equality checks.
- The `A-Z` check is a single unsigned compare on `c - 65`.
- The lowercase conversion adds 0 or 32 to each character, so no conditional branch or conditional store is needed.
- The word count update is computed with bitwise operations.
- The loop has one conditional branch per character.

## Results Summary

- The **optimized assembly** is the fastest of the three versions.
- The **unoptimized assembly** is about the same as the C version or slightly slower. Its structure mirrors the C code, and GCC at `-O2` already restructures the loop and simplifies the comparisons, so the hand-written version has no advantage.
- The optimized version is faster because it removes branches from the per-character loop, which the unoptimized version still executes.

Exact timings vary between runs, so run the program a few times when comparing.

## Notes

- All timings come from QEMU emulation, not physical RISC-V hardware. Emulated timings may not match real hardware, especially for branch-related behavior.
- The C and unoptimized assembly versions treat space, `\n`, `\t`, and `\r` as whitespace. The optimized version treats every character with an ASCII value of 32 or below as whitespace, which includes those four plus other control characters.
