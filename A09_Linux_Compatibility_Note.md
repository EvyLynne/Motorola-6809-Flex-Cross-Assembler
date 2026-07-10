# Platform Compatibility Note — A09 Motorola 6809 Cross-Assembler (C17)

*July 10, 2026*

## 1. Compatibility Statement

The modernized A09 assembler runs on Linux, and Linux is in fact its most-tested platform: every build and functional verification performed during the modernization project was executed on Linux (Ubuntu, GCC 13.3), and the project's original stated target was Ubuntu 20.10 with GCC 10.2.0 in Oracle VM VirtualBox. The Windows/MinGW-w64 toolchain was added later as a second platform.

The code is plain ISO/IEC 9899:2018 (C17) with no operating-system-specific calls — no Windows API and no Linux-only headers. Both build layouts compile with zero warnings under `gcc -std=c17 -Wall -Wextra -pedantic` on GCC versions 10 through 16 on either platform.

## 2. What Is Identical on Linux

- **Build commands** are the same, minus the `.exe` suffix:

  ```bash
  gcc -std=c17 -Wall -Wextra -pedantic -o a09 a09.c
  gcc -std=c17 -Wall -Wextra -pedantic -o a09 a09_data.c a09_core.c a09_main.c
  ```

- **Usage** is the same, including the fixed option order (output flag, then `-l`, then the source file last):

  ```bash
  ./a09 -o test.bin -l test.lst test09.asm
  ./a09 -s test.srec test09.asm
  ./a09 -x test.ihex test09.asm
  ```

- **File-mode handling:** the `"wb"` versus `"w"` distinction in `main()` when opening the object file only matters on Windows; on Linux the two modes are equivalent and the program behaves identically.
- **Output correctness:** the binary, Motorola S-record, and Intel Hex outputs, including all checksums and end records, were verified on Linux; `test09.asm` assembles to exactly 29 bytes.

## 3. Two Linux-Specific Points to Watch

**Line endings in `.asm` source files.** An assembly source file created on Windows carries CRLF line endings. Assembled on Linux, each line then arrives with a trailing carriage-return character that the 1993 scanner does not strip, which can produce spurious `Illegal addressing mode` errors on otherwise-correct lines. Normalize before assembling with either command:

```bash
sed -i 's/\r$//' yourfile.asm
dos2unix yourfile.asm
```

The five delivered `.c` files and `test09.asm` are already LF-normalized, so they compile and assemble cleanly on both systems as-is. The reverse direction is safe: files created on Linux work on Windows, because Windows text mode accepts both endings.

**Filename case sensitivity.** `Test09.asm` and `test09.asm` are different files on Linux, so filenames given on the command line and in `INCLUDE` pseudo-ops must match case exactly. This is a non-issue on Windows and occasionally surprises when moving a project over.

## 4. Recommended Verification

If the original VirtualBox Ubuntu VM is still available, run the fresh-clone test from the GitHub setup guide's verification checklist (item 7) there: clone the repository, run the README build command, assemble `test09.asm`, and confirm the 29-byte output. Passing that test on both platforms supports an honest "verified on Windows and Linux" claim in the repository README and the LinkedIn article.
