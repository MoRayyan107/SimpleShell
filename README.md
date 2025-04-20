# CS210 - Simple Shell

![Language: C](https://img.shields.io/badge/language-C-blue)
![Status: Stage 8 Complete](https://img.shields.io/badge/status-Stage%208-brightgreen)
![Score: 82%](https://img.shields.io/badge/Score-82%25-blueviolet)

A simplified shell for Unix-like systems written in **C**.

[Compiled on Linux based OS]

This project was developed for the CS210: Computer Systems and Architecture course. It includes functionality for basic shell commands, aliasing, history tracking, and persistent environment handling.

---

## 👨‍💻 Written By

- [Mohammad Rayyan Adhoni](https://github.com/MoRayyan107)
- Delvin Santhosh
- Mohamed Sharif
- Kyle Slavin
- Nathan Lewis

---

## 📄 Overview

This shell supports the following built-in and custom commands:

| **Command**              | **Description**                    |
| ------------------------ | ---------------------------------- |
| `echo`                   | Displays user input on screen      |
| `getpath`                | Display current environment `PATH` |
| `setpath <path>`         | Set a new `PATH` value             |
| `cd <dir>`               | Change working directory           |
| `pwd`                    | Print current directory            |
| `history`                | Show recent command history        |
| `!!`                     | Invoke the most recent command     |
| `!<n>`                   | Invoke the *nth* newest command    |
| `!-<n>`                  | Invoke the *nth* oldest command    |
| `alias`                  | Show all current aliases           |
| `alias <name> <command>` | Create or update an alias          |
| `unalias <name>`         | Remove a specified alias           |
| `exit`                   | Exit the shell safely              |

> 💡 *Aliases and history are stored persistently in your home directory as `.aliases` and `.hist_list`.*

---

## ✅ Development Progress

| Stage | Description                               | Status      | Lead Contributor       |
| ----- | ----------------------------------------- | ----------- | ---------------------- |
| 1     | Shell prompt and user input               | 🟩 Complete | Mohammad Rayyan Adhoni |
| 2     | External command execution                | 🟩 Complete | Mohamed Sharif         |
| 3     | Environment variable (`PATH`) management  | 🟩 Complete | Kyle Slavin            |
| 4     | `cd` command with HOME fallback           | 🟩 Complete | Delvin Santhosh        |
| 5     | Command history + invocation shortcuts    | 🟩 Complete | Mohammad Rayyan Adhoni |
| 6     | Persistent history file                   | 🟩 Complete | Mohamed Sharif         |
| 7     | Alias system (minor bug in `cd` aliasing) | 🟧 Mostly   | Nathan Lewis           |
| 8     | Persistent alias file support             | 🟩 Complete | Rayyan & Sharif        |
| 9     | Recursive aliasing + aliasing history     | 🟥 Pending  | -                      |
---

## 🛠️ Technologies Used

- **Language:** C
- **Compiler:** GCC
- **Core Concepts:** Process management, file I/O, environment variables, memory handling

---

## 🔮 Future Enhancements

- 🔁 Recursive alias resolution
- 🔄 Cycle detection for aliases
- 📤 I/O redirection & piping
- 🧠 Tab-completion with `readline()`
- 🕹 Background execution (`&`)

---

## 📁 File Storage

- `~/.hist_list` — History file
- `~/.aliases` — Alias file

---
## 👨‍💻 How To Use?
This shell is programmed in Linux based OS

Download the `zip file` of this repository and extract it.

In your terminal guide to the downloaded file and type this
```
make
```

then type
```
./Shell
```
And voilà thats a running shell

To throw away old build files, then type
```
make clean
```
Or to rebuild with the new files, then type
```
make rebuild
```

---

## 📜 License

This project was developed as part of a university assignment and is intended for educational use only.

---
