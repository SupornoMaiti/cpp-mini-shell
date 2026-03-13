# cpp-mini-shell 🐚

A minimal Unix-like shell built from scratch in **C++** to deeply understand Operating Systems internals — process control, system calls, and I/O management.

> Built as a learning project while studying OS concepts. No external libraries — just raw Linux system calls.

---

## ✨ Features

- ✅ **Command Execution** — runs standard Linux programs (`ls`, `grep`, `pwd`, `cat`, etc.)
- ✅ **Piping** — chain commands with `|` (supports multiple pipes)
- ✅ **Output Redirection** — `>` (overwrite) and `>>` (append) stdout to file
- ✅ **Input Redirection** — `<` read stdin from file
- ✅ **Quote Handling** — `echo "Hello World"` treated as single token
- ✅ **Colored Prompt** — displays `user@hostname:cwd$` with ANSI colors
- ✅ **Built-in Commands** — `cd` (with error handling), `exit`
- ✅ **Process Management** — safe child process creation via `fork` + `waitpid`

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
exec_in_child()   ──►  Applies <, >, >> redirections via dup2, then execvp()
    │
    ├── Pipeline:  Creates n-1 pipes, forks n children, wires stdin/stdout
    └── Parent:    Closes all pipe ends, waitpid() for all children
```

---

## 📚 Core OS Concepts Applied

| Concept                 | System Call / Flag                        |
| ----------------------- | ----------------------------------------- |
| Process Creation        | `fork()`                                  |
| Program Execution       | `execvp()`                                |
| Process Synchronization | `waitpid()`                               |
| Output Redirection      | `dup2()`, `open()`, `O_WRONLY`, `O_TRUNC` |
| Append Redirection      | `O_APPEND`                                |
| Input Redirection       | `dup2()`, `open()`, `O_RDONLY`            |
| Piping                  | `pipe()`                                  |
| Directory Navigation    | `chdir()`                                 |
| Environment Access      | `getenv()`                                |
| Hostname Resolution     | `gethostname()`                           |
| Working Directory       | `getcwd()`                                |

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

## 🏛️ Code Architecture

```
Shell class
├── tokenize()           — string → vector<string>, handles quotes
├── split_by_pipe()      — vector<string> → vector<vector<string>>
├── redirection()        — parses <, >, >> and strips from tokens
├── tokens_to_c_pointers() — vector<string> → char*[] for execvp
├── handle_cd()          — built-in cd command
├── exec_in_child()      — applies redirections + execvp (reusable core)
├── execute_command()    — single command: fork + exec_in_child + waitpid
├── execute_pipeline()   — N commands: N-1 pipes + N forks + exec_in_child
├── get_prompt()         — builds colored user@host:cwd$ prompt
└── run()                — main REPL loop
```

---

## 🗺️ Roadmap

- [x] Basic command execution
- [x] Output redirection (`>`)
- [x] Append redirection (`>>`)
- [x] Input redirection (`<`)
- [x] Colored prompt with user, host, cwd
- [x] Built-in `cd` and `exit`
- [x] Quote handling
- [x] Piping (`|`) with multiple commands
- [x] Pipe + redirection combined
- [ ] Signal handling (`Ctrl+C`, `Ctrl+Z`)
- [ ] Command history (arrow keys)
- [x] `~` home directory shorthand in prompt

---

## 🧠 Why I Built This

Most developers use a shell every day without knowing what happens under the hood. Building one from scratch forced me to understand exactly how processes are created, how file descriptors work, how pipes connect processes, and how the OS manages execution — concepts that map directly to GATE OS topics and systems programming interviews.

---

## 📜 License

MIT License — feel free to use, fork, and learn from it.
