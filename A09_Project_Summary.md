# Modernization to ISO/IEC 9899:2018 (C17) — Project Summary

*July 10, 2026*

## Table of Contents

- [1. Summary Overview](#1-summary-overview)
  - [1.1 What Was Assembled from Where](#11-what-was-assembled-from-where)
  - [1.2 C17 Fixes Applied](#12-c17-fixes-applied)
- [2. Source of the Cross-Assembler (Documentation)](#2-source-of-the-cross-assembler-documentation)
  - [2.1 Origin and Primary Source](#21-origin-and-primary-source)
  - [2.2 Alternative Complete 6809 Assemblers (for reference)](#22-alternative-complete-6809-assemblers-for-reference)
  - [2.3 Compiler Toolchain Used (GCC for Windows)](#23-compiler-toolchain-used-gcc-for-windows)
- [3. Compiling and Running — Command Reference](#3-compiling-and-running--command-reference)
  - [3.1 Compile (single merged file)](#31-compile-single-merged-file)
  - [3.2 Compile (three-file split with shared header)](#32-compile-three-file-split-with-shared-header)
  - [3.3 Run — command syntax](#33-run--command-syntax)
  - [3.4 Run — worked examples](#34-run--worked-examples)
  - [3.5 Inspecting the Output (Windows)](#35-inspecting-the-output-windows)
- [4. Guide to the Source Files](#4-guide-to-the-source-files)

## 1. Summary Overview

This document includes 2 versions of the cross compiler: one complete executable file and 3 separate files with a shared header.

- **Merged version:** `a09.c` (1,475 lines) — a single self-contained file containing the complete assembler.
- **Split version:** `a09.h` (shared header: constants, macros, struct types, extern globals, all prototypes) plus three `.c` files — `a09_data.c` (opcode/register/symbol tables and global state), `a09_core.c` (all 49 functions), and `a09_main.c` (`main`).
- **Both versions compile with zero warnings under** `-std=c17 -Wall -Wextra -pedantic` on GCC 13, which is stricter than the GCC 10.2.0 originally targeted, so builds on that toolchain are clean as well.
- **Both versions were functionally tested by assembling a real 6809 program:** binary, Motorola S-record, and Intel Hex outputs are all correct, every hex-record checksum was mathematically verified, and the merged and split builds produce byte-identical output in all three modes plus the listing file.

### 1.1 What Was Assembled from Where

- **The uploaded fragments** (`header.c`, `defines.c`, `new_4.c`) were used verbatim as the authoritative function bodies — including the Intel Hex extensions (`-x` option, outmode 2, `flushihex`/`outihex`), which do not exist in the base sbc09 version.
- **The missing pieces** (structs, the 168-entry 6809 opcode table, register table, error messages, ~40 globals) came from the original GPL v2 `a09.c` in the sbc09 GitHub repository, and the original copyright and license header is preserved in every file.

### 1.2 C17 Fixes Applied

- Added the standard `#include`s and a complete prototype block — fixes the implicit declarations for the recursive `scanexpr()` and mutually recursive `processfile()`/`pseudoop()`, which are hard errors on modern compilers.
- `processfile()` changed from `int` (it never returned a value — undefined behavior under C99 and later) to `void`.
- `main()`: the `getchar()` result is now stored in an `int` for correct EOF handling, and `return 0` was added.
- All empty parameter lists `()` converted to `(void)`; all `ctype.h` calls given `(unsigned char)` casts.
- `RESOLVECAT` rewritten as a clean brace-block macro — the original relied on a comment spliced into the macro body via line continuation.
- **Real bug fixed:** `hexbuffer` enlarged from 16 to 32 bytes. `outihex()` fills up to 32 bytes before flushing, which overflowed the original 16-byte buffer; the 29-byte Intel Hex test record exercised exactly this path.
- **Preserved bug-for-bug and commented in place:** the missing `break` after `case '|'` in `scanexpr` (present in the 1993/94 original too), and the unbounded `strcpy` calls in `getoptions`.

## 2. Source of the Cross-Assembler (Documentation)

### 2.1 Origin and Primary Source

The uploaded fragments were identified as extracts of `a09.c`, a Motorola 6809 cross-assembler created 1993–1994 by L.C. (Lennart) Benschop, originally posted to the Usenet newsgroup alt.sources on November 3, 1993.

- **Complete maintained source:** the sbc09 project on GitHub — <https://github.com/6809/sbc09> — file `src/a09.c` (1,338 lines). This repository contains an assembler and simulator for the Motorola M6809 processor.
- **License:** GNU General Public License version 2 (GPL v2); copyleft 1994–2014 by the sbc09 team. The original copyright header is retained in all delivered files.
- **Verification:** the downloaded original was compared against the fragments and matched line-for-line in all shared functions; the fragments cover all 49 functions plus `main()`. The Intel Hex output support in the fragments (outmode 2, `-x` option) is a later fork addition not present in the sbc09 base, and was preserved from the fragments themselves.

### 2.2 Alternative Complete 6809 Assemblers (for reference)

- **Arakula/A09 (GitHub, C)** — a complete macro assembler built on the same Benschop core, covering 6800/6801/6809/6309/68HC11, with binary, Intel Hex, Motorola S1, and Flex9 output.
- **asm6809 (6809.org.uk, C)** — a modern, actively maintained 6809/6309 assembler (v2.17, June 2025); outputs include raw binary, DragonDOS, CoCo RS-DOS, Motorola SREC, and Intel HEX; GPL v3.
- **spc476/a09 (GitHub, C)** — an unrelated modern 6809 assembler with extensive warnings and testing directives.
- **as9 (C)** — the official Motorola assembler per the 1981 MC6809 Programming Manual, ported to build on Linux/GCC.
- **LWTOOLS** — includes LWASM, the assembler most commonly used by today's CoCo/Dragon retro-development community.

### 2.3 Compiler Toolchain Used (GCC for Windows)

- **Toolchain:** WinLibs standalone build of GCC + MinGW-w64, downloaded from <https://winlibs.com> — a free C and C++ compiler for Microsoft Windows requiring no installer and no administrator rights.
- **Version installed:** GCC 16.1.0 (POSIX threads) + MinGW-w64 14.0.0 with the UCRT runtime (Win64 ZIP archive), extracted into the user profile and added to the user-level PATH.
- **Cross-check:** the delivered sources were also compiled on Linux under GCC 13.3.0 with identical flags and produced zero warnings, confirming portability across GCC 10–16.

## 3. Compiling and Running — Command Reference

### 3.1 Compile (single merged file)

```bash
gcc -std=c17 -Wall -Wextra -pedantic -o a09.exe a09.c
```

### 3.2 Compile (three-file split with shared header)

```bash
gcc -std=c17 -Wall -Wextra -pedantic -o a09.exe a09_data.c a09_core.c a09_main.c
```

`a09.h` must be in the same folder. Both builds should complete silently; any warning is unexpected and worth reporting. On Linux, omit the `.exe` suffix.

### 3.3 Run — command syntax

```text
a09 [-o objname | -s objname | -x objname] [-l listname] srcname
```

- **Output format options:** binary image (`-o`), Motorola S-records (`-s`), or Intel Hex (`-x`). One output format per run; all three write to the same object filename.
- **Order matters:** the option parser checks flags in one fixed sequence (`-o`, then `-s`, then `-x`, then `-l`), and the source filename must come last. Running `-l` before `-o` produces `Cannot open source file -o`. This is preserved 1993 behavior.

### 3.4 Run — worked examples

Assemble to binary with a listing:

```bash
a09.exe -o test.bin -l test.lst test09.asm
```

Assemble to Motorola S-records:

```bash
a09.exe -s test.srec test09.asm
```

Assemble to Intel Hex:

```bash
a09.exe -x test.ihex test09.asm
```

Expected results for `test09.asm`: silent completion (both passes clean); `test.bin` is exactly 29 bytes beginning `10 CE 04 00` (`LDS #$400`); the listing shows each line's address and generated bytes plus a symbol table containing `DATA`, `LOOP`, `LOOP2`, and `START`.

### 3.5 Inspecting the Output (Windows)

Display the listing in the console:

```bat
type test.lst
```

Hex-dump the binary machine code:

```bat
certutil -dumphex test.bin
```

> **Note:** the 29 bytes produced are Motorola 6809 machine code and cannot execute on an Intel PC directly; running the assembled program requires a 6809 emulator such as the sbc09 project's own v09 simulator, XRoar, or MAME.

## 4. Guide to the Source Files

| File Name | Description | Usage |
|---|---|---|
| `a09.c` | Single merged source file (1,475 lines). Complete, self-contained C17 assembler: attribution header, includes, constants, macros, opcode/register/symbol tables, global state, prototypes, all 49 functions, and `main()`. | Standalone build. Compile this one file alone; do not combine with the split files. Use when you want one-file simplicity. |
| `a09.h` | Shared header for the three-file split: include guard, standard `#include`s, constants (`NLABELS`, `MAXIDLEN`, `FNLEN`, `LINELEN`), the `EXITEVAL` / `RESOLVECAT` / `RESTORE` macros, struct types (`oprecord`, `symrecord`, `regrecord`), extern declarations for all globals and tables, and prototypes for all 50 functions. | Not compiled directly. `#include`d by each of the three split `.c` files; must sit in the same folder when compiling the split build. |
| `a09_data.c` | Tables and global state: the 168-entry 6809 opcode table (`optable`), register table (`regtable`), symbol table storage, error message strings, and every global variable definition, including the enlarged 32-byte Intel Hex buffer. | Compiled as part of the split build. Edit here to add mnemonics or change table data. |
| `a09_core.c` | The working machinery (1,114 lines): scanner, recursive-descent expression evaluator, operand/addressing-mode analysis, code generators for every instruction category, pseudo-op handling, listing/output writers (binary, S-record, Intel Hex), option parsing, and the two-pass file driver. | Compiled as part of the split build. Edit here to change assembler behavior. Full C17 revision notes are in this file's header comment. |
| `a09_main.c` | `main()` only: runs pass 1, reports pass-1 errors and asks whether to continue, opens list/object files (binary vs. text mode as appropriate), runs pass 2, writes end records (S9 or Intel Hex EOF), and closes files. | Compiled as part of the split build. Program entry point. |
| `test09.asm` | 6809 test program (not C source; supporting file). Exercises immediate, indexed-with-postincrement, and relative-branch addressing plus `fcb`/`fdb`/`fcc` data pseudo-ops. Assembles to exactly 29 bytes. | Input for verifying the assembler: `a09.exe -o test.bin -l test.lst test09.asm` |

*The five source files represent the same program two ways: compile `a09.c` alone, or compile the three split `.c` files together (with `a09.h` present). Do not mix the two sets in one build — every function would be defined twice.*
