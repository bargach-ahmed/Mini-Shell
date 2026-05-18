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

## 4. Robust Error Handling
A good shell catches bad input without crashing. We built custom wrappers in `utils.c` to make sure of this.

* **Type a command that doesn't exist:**
  ```bash
  asdfghjkl1234
  ```
  *(Expected behavior: It prints an error message like "OrbitShell: No such file or directory" and gives you the prompt back. It will NOT crash!)*

* **Try `cd` without an argument:**
  ```bash
  cd
  ```
  *(Expected behavior: It gracefully prints "minishell: expected argument to cd".)*

---

## 5. The "Breaking Orbit" Custom Exit Animation
Test the custom ascii animation we built into `shell_exit`!

* **Exit the shell normally:**
  ```bash
  exit
  ```
  *(Expected behavior: The terminal will print "Breaking Orbit...", display a 3-second spaceship loading animation, and then terminate).*

* **Alternative Exit Method (EOF):**
  Press `Ctrl+D` on your keyboard.
  *(Expected behavior: The shell recognizes the End-Of-File signal from `getline` and triggers the same exact exit animation!)*
