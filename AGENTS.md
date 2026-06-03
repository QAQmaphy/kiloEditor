# kiloEditor — AGENTS.md

## Build & Run

```bash
make          # builds `kilo` from `kilo.c` (Wall -Wextra -pedantic -std=c99)
./kilo        # launch the editor
```

## Quitting

Press **Ctrl+Q** to exit (the README incorrectly says `q`). The code uses `CTRL_KEY('q')` = `0x11`.

## Code

- Single file (`kilo.c`, 194 lines). No headers, no libraries beyond POSIX.
- Terminal raw mode via `<termios.h>`, window size via `ioctl(TIOCGWINSZ)`.

## Binary

`kilo` (ELF) is tracked in git. There is no `.gitignore`.
