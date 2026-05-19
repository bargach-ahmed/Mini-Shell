# OrbitShell Testing Guide

Once you have compiled your shell (by running `make` in your Ubuntu terminal) and launched it (by running `./minishell`), use the following commands to test its capabilities!

---

## 1. System Commands & Arguments
OrbitShell automatically finds and executes standard Unix commands using `execvp` and `fork`. It also dynamically parses any arguments you provide without any fixed memory limits.

* **See your current path:**
  ```bash
  pwd
  ```

* **List files with formatting flags:**
  ```bash
  ls -la
  ```

* **Read the source code:**
  ```bash
  cat utils.c
  ```

* **Print a string with spaces (tests the dynamic tokenizer):**
  ```bash
  echo "OrbitShell is working!"
  ```

---

## 2. The `cd` Built-in (Change Directory)
Unlike `ls` or `pwd`, `cd` is a built-in command that we programmed in `builtins.c` to directly modify the parent process environment.

* **Go back one directory:**
  ```bash
  cd ..
  ```
  *(Notice how your futuristic `OrbitShell:` prompt updates to show the new folder dynamically!)*

* **Navigate deep into a system directory:**
  ```bash
  cd /tmp
  ```

* **Go to your home directory:**
  ```bash
  cd
  cd ~
  ```

* **Jump back to the previous directory:**
  ```bash
  cd -
  ```

* **Navigate to the Windows C: drive (WSL only):**
  ```bash
  cd /mnt/c
  ```

---

## 3. The `env` Built-in (Environment Variables)
This prints out all the background variables loaded into your Linux session by iterating through the global `environ` array.

* **Print the environment:**
  ```bash
  env
  ```

---

## 4. Redirection and Pipelines
OrbitShell can redirect standard input/output and connect commands with pipes.

* **Write output to a file:**
  ```bash
  echo hello > output.txt
  cat output.txt
  ```

* **Append output to a file:**
  ```bash
  echo again >> output.txt
  cat output.txt
  ```

* **Read command input from a file:**
  ```bash
  wc -w < output.txt
  ```

* **Pipe one command into another:**
  ```bash
  echo hello | wc -w
  ```

---

## 5. Signal Handling
OrbitShell keeps the parent shell alive when you press `Ctrl+C`.

* **Interrupt a long-running child process:**
  ```bash
  sleep 10
  ```
  Press `Ctrl+C`.
  *(Expected behavior: `sleep` stops and OrbitShell gives you a fresh prompt.)*

---

## 6. Robust Error Handling
A good shell catches bad input without crashing. We built custom wrappers in `utils.c` to make sure of this.

* **Type a command that doesn't exist:**
  ```bash
  asdfghjkl1234
  ```
  *(Expected behavior: It prints an error message like "OrbitShell: No such file or directory" and gives you the prompt back. It will NOT crash!)*

* **Try changing to an invalid directory:**
  ```bash
  cd /this/path/does/not/exist
  ```
  *(Expected behavior: It gracefully prints a `minishell cd` error and gives you the prompt back.)*

---

## 7. The "Breaking Orbit" Custom Exit Animation
Test the custom ascii animation we built into `shell_exit`!

* **Exit the shell normally:**
  ```bash
  exit
  ```
  *(Expected behavior: The terminal will print "Breaking Orbit...", display a 3-second spaceship loading animation, and then terminate).*

* **Alternative Exit Method (EOF):**
  Press `Ctrl+D` on your keyboard.
  *(Expected behavior: The shell recognizes the End-Of-File signal from `getline` and triggers the same exact exit animation!)*
