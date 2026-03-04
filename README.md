# cpp-mini-shell 🐚

A minimal Unix-like shell built from scratch in **C++** to deeply understand Operating Systems internals — process control, system calls, and I/O management.

> Built as a learning project while studying OS concepts. No external libraries — just raw Linux system calls.

---

## ✨ Features

- ✅ **Command Execution** — runs standard Linux programs (`ls`, `grep`, `pwd`, `cat`, etc.)
- ✅ **Output Redirection** — supports `>` (stdout to file)
- ✅ **Colored Prompt** — displays `user@hostname:cwd$` with ANSI colors
- ✅ **Built-in Commands** — `cd`, `exit`
- ✅ **Process Management** — safe child process creation via `fork` + `wait`
- 🔧 **Piping** *(in progress)* — chaining commands with `|`

---

## ⚡ Quick Start

```bash
# Clone the repo
git clone https://github.com/SupornoMaiti/cpp-mini-shell.git
cd cpp-mini-shell

# Build with CMake
mkdir build && cd build
cmake ..
make

# Or compile directly
g++ src/main.cpp -o shell

# Run
./shell
```

---

## 🧪 Usage Examples

```bash
# Basic command
suporno@arch:~$ ls -la

# Output redirection
suporno@arch:~$ ls > output.txt

# Change directory
suporno@arch:~$ cd /home/user

# Exit
suporno@arch:~$ exit
```

---

## 🏗️ How It Works

```
User Input
    │
    ▼
tokenize()  ──►  Splits input into command + arguments
    │
    ▼
handle_cd()  ──►  Checks for built-in cd, runs chdir()
    │
    ▼
redirection()  ──►  Detects '>' and extracts filename, strips from args
    │
    ▼
fork()  ──►  Creates child process
    │
    ├── Child:   dup2() to redirect stdout → file (if needed), then execvp()
    └── Parent:  wait() for child to finish
```

---

## 📚 Core OS Concepts Applied

| Concept | System Call Used |
|---|---|
| Process Creation | `fork()` |
| Program Execution | `execvp()` |
| Process Synchronization | `wait()` |
| Output Redirection | `dup2()`, `open()`, `close()` |
| Directory Navigation | `chdir()` |
| Environment Access | `getenv()` |
| Hostname Resolution | `gethostname()` |
| Working Directory | `getcwd()` |
| Piping *(WIP)* | `pipe()` |

---

## 📁 Project Structure

```
cpp-mini-shell/
├── src/
│   └── main.cpp       # Core shell logic
├── CMakeLists.txt     # CMake build config
├── LICENSE
├── .gitignore
└── README.md
```

---

## 🗺️ Roadmap

- [x] Basic command execution
- [x] Output redirection (`>`)
- [x] Colored prompt with user, host, cwd
- [x] Built-in `cd` and `exit`
- [ ] Input redirection (`<`) and append (`>>`)
- [ ] Piping (`|`) — *in progress*
- [ ] Background processes (`&`)
- [ ] Signal handling (`Ctrl+C`, `Ctrl+Z`)
- [ ] Command history

---

## 🧠 Why I Built This

Most developers use a shell every day without knowing what happens under the hood. Building one from scratch forced me to understand exactly how processes are created, how file descriptors work, and how the OS manages execution — concepts that map directly to GATE OS topics and systems programming interviews.

---

## 📜 License

MIT License — feel free to use, fork, and learn from it.
