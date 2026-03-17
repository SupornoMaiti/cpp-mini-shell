# cpp-mini-shell 🐚

A minimal Unix-like shell built from scratch in **C++** to deeply understand Operating Systems internals — process control, system calls, and I/O management.

> Built as a learning project while studying OS concepts. No external libraries except `readline` — just raw Linux system calls.

---

## 📌 Project Status

**Level 1 & 2 Complete — Intentionally Scoped**

This project is intentionally scoped. The goal was to deeply understand OS internals through building — not to recreate bash.

**What's implemented:** Core shell features including process management, I/O redirection, piping, signal handling, and command history — all built from scratch using raw Linux system calls.

**What's next:** Moving to a new systems project to explore different OS concepts. This codebase will remain as a reference for the concepts learned.

> If you're learning OS internals, feel free to fork and build on top of this.

---

## ✨ Features

- ✅ **Command Execution** — runs standard Linux programs (`ls`, `grep`, `pwd`, `cat`, etc.)
- ✅ **Piping** — chain commands with `|` (supports multiple pipes)
- ✅ **Output Redirection** — `>` (overwrite) and `>>` (append) stdout to file
- ✅ **Input Redirection** — `<` read stdin from file
- ✅ **Quote Handling** — `echo "Hello World"` treated as single token
- ✅ **Signal Handling** — `Ctrl+C` kills child only, `Ctrl+Z` stops child only — shell always survives
- ✅ **Command History** — arrow keys to navigate history, `Ctrl+R` reverse search, persists across sessions
- ✅ **Colored Prompt** — displays `user@hostname:~` with ANSI colors and `~` home shorthand
- ✅ **Tilde Expansion** — `cd ~/Projects` works correctly
- ✅ **Built-in Commands** — `cd` (with error handling), `exit`
- ✅ **Process Management** — safe child process creation via `fork` + `waitpid`

---

## ⚡ Quick Start

```bash
# Clone the repo
git clone https://github.com/SupornoMaiti/cpp-mini-shell.git
cd cpp-mini-shell

# One command setup (installs dependencies + builds)
chmod +x scripts/setup.sh
./scripts/setup.sh

# Or manually
sudo apt install libreadline-dev
mkdir build && cd build
cmake ..
make

# Run
./Shell_Project
```

---

## 🧪 Usage Examples

```bash
# Basic command
suporno@arch:~$ ls -la

# Piping
suporno@arch:~$ ls | grep .cpp
suporno@arch:~$ ls | grep .cpp | sort

# Output redirection
suporno@arch:~$ ls > output.txt

# Append redirection
suporno@arch:~$ echo "hello" >> log.txt

# Input redirection
suporno@arch:~$ sort < input.txt

# Combined — pipe with redirection
suporno@arch:~$ ls | grep .cpp > output.txt

# Quote handling
suporno@arch:~$ echo "Hello World"

# Tilde expansion
suporno@arch:~$ cd ~/Projects
suporno@arch:~/Projects$

# Command history
suporno@arch:~$ ls        # run command
suporno@arch:~$           # press ↑ → ls reappears
suporno@arch:~$           # Ctrl+R → reverse search

# Signal handling — shell survives both
suporno@arch:~$ sleep 10  # Ctrl+C → kills sleep, shell continues
suporno@arch:~$ sleep 10  # Ctrl+Z → stops sleep, shell continues

# Exit
suporno@arch:~$ exit
```

---

## 🏗️ How It Works

```
User Input (via readline)
    │
    ▼
tokenize()        ──►  Splits input into tokens, handles quoted strings
    │
    ▼
split_by_pipe()   ──►  Splits token list by | into separate commands
    │
    ▼
single command?   ──►  handle_cd() or execute_command()
multiple commands?──►  execute_pipeline()
    │
    ▼
exec_in_child()   ──►  Restores signal defaults, applies redirections, execvp()
    │
    ├── Pipeline:  Creates n-1 pipes, forks n children, wires stdin/stdout
    └── Parent:    Closes all pipe ends, waitpid(WUNTRACED) for all children
```

---

## 📚 Core OS Concepts Applied

| Concept                  | System Call / Flag                        |
| ------------------------ | ----------------------------------------- |
| Process Creation         | `fork()`                                  |
| Program Execution        | `execvp()`                                |
| Process Synchronization  | `waitpid()`, `WUNTRACED`                  |
| Signal Handling          | `sigaction()`, `SIGINT`, `SIGTSTP`        |
| Output Redirection       | `dup2()`, `open()`, `O_WRONLY`, `O_TRUNC` |
| Append Redirection       | `O_APPEND`                                |
| Input Redirection        | `dup2()`, `open()`, `O_RDONLY`            |
| Piping                   | `pipe()`                                  |
| Directory Navigation     | `chdir()`                                 |
| Environment Access       | `getenv()`                                |
| Hostname Resolution      | `gethostname()`                           |
| Working Directory        | `getcwd()`                                |
| Command History          | `readline`, `add_history`, `read_history` |

---

## 📁 Project Structure

```
cpp-mini-shell/
├── src/
│   └── main.cpp           # Core shell logic
├── scripts/
│   └── setup.sh           # One command setup for all platforms
├── CMakeLists.txt         # CMake build config
├── LICENSE
├── .gitignore
└── README.md
```

---

## 🏛️ Code Architecture

```
Shell class
├── tokenize()             — string → vector<string>, handles quotes
├── split_by_pipe()        — vector<string> → vector<vector<string>>
├── redirection()          — parses <, >, >> and strips from tokens
├── tokens_to_c_pointers() — vector<string> → char*[] for execvp
├── expand_tilde()         — expands ~ to $HOME in paths
├── home_dir()             — replaces /home/user prefix with ~ in prompt
├── handle_cd()            — built-in cd with tilde expansion
├── exec_in_child()        — restores signals, applies redirections, execvp
├── execute_command()      — single command: fork + exec_in_child + waitpid
├── execute_pipeline()     — N commands: N-1 pipes + N forks + exec_in_child
├── get_prompt()           — builds colored user@host:~path$ prompt
└── run()                  — signal setup, history setup, main REPL loop
```

---

## 🔒 Signal Handling Design

```
Shell (run())         →  SIGINT ignored,  SIGTSTP ignored
exec_in_child()       →  SIGINT restored, SIGTSTP restored

Result:
  Ctrl+C  →  kills   only the foreground child, shell survives
  Ctrl+Z  →  stops   only the foreground child, shell survives
```

---

## 📜 History Design

```
startup  →  read_history()   loads ~/.myshell_history → RAM
running  →  add_history()    adds each command → RAM (deduplicated)
exit     →  write_history()  saves RAM → ~/.myshell_history

Features:
  ↑ / ↓     → navigate through previous commands
  Ctrl+R    → reverse search through history
  limit     → 1000 entries (stifle_history)
  persists  → survives shell restart
```

---

## 🗺️ Roadmap

- [x] Basic command execution
- [x] Output redirection (`>`)
- [x] Append redirection (`>>`)
- [x] Input redirection (`<`)
- [x] Colored prompt with user, host, cwd
- [x] `~` home directory shorthand in prompt
- [x] Tilde expansion in `cd` and paths
- [x] Built-in `cd` and `exit`
- [x] Quote handling
- [x] Piping (`|`) with multiple commands
- [x] Pipe + redirection combined
- [x] Signal handling (`Ctrl+C`, `Ctrl+Z`)
- [x] Command history with persistence

---

## 🧠 Why I Built This

Most developers use a shell every day without knowing what happens under the hood. Building one from scratch forced me to understand exactly how processes are created, how file descriptors work, how pipes connect processes, how signals are delivered, and how the OS manages execution.

This project is now feature-complete at its intended scope — a working, usable shell that demonstrates deep understanding of how Unix shells work under the hood.

---

## 📜 License

MIT License — feel free to use, fork, and learn from it.