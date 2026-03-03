# cpp-mini-shell 🐚

A minimal Linux shell written in C++ to learn Operating Systems internals.

**Core Concepts:** Process Control (`fork`, `exec`, `wait`), System Calls, and C++ Memory Management.

## ⚡ Quick Start

```bash
g++ src/main.cpp -o shell
./shell

```

## 🛠️ What It Does

1. **Reads** commands from your terminal.
2. **Parses** the input into arguments.
3. **Forks** a child process to run the command safely.
4. **Executes** standard Linux programs (like `ls`, `grep`, `pwd`).
