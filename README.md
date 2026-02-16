# Mini Unix Shell (C++)

A lightweight Unix-like shell implemented in C++ using POSIX system calls.  
This project demonstrates core operating system concepts including process creation, inter-process communication, and file descriptor management.

---

## Features

- Execute external commands
- Built-in commands: `cd`, `exit`
- Background process execution (`&`)
- Input redirection (`<`)
- Output redirection (`>`)
- Single pipe support (`|`)
- Linux-compatible implementation

---

## System Calls Used

- `fork()`
- `execvp()`
- `waitpid()`
- `pipe()`
- `dup2()`
- `open()`
- `chdir()`

---

## Concepts Demonstrated

- Process lifecycle management
- Parent-child synchronization
- Inter-process communication (IPC)
- File descriptor duplication and redirection
- Command parsing
- Basic shell architecture

---

## Project Structure

