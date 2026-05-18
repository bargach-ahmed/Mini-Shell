# OrbitShell Project

This is a custom Unix mini-shell built in C. It features a highly modular, multi-file architecture (inspired by educational systems programming projects), dynamic memory management, POSIX system calls for process execution, built-in commands (`exit`, `cd`, `env`), and a futuristic aesthetic prompt.

## Prerequisites (For Windows Users)

Since this project relies heavily on Unix-specific POSIX system calls (`fork`, `execvp`, `wait`), **it cannot be compiled natively on a standard Windows environment (like MSVC or MinGW)** without modifications.

To compile and run this project on Windows, you must use **Windows Subsystem for Linux (WSL)**.

### Step 1: Install WSL (Ubuntu)
1. Open PowerShell or Command Prompt as Administrator.
2. Run the following command:
   ```bash
   wsl --install
   ```
3. Restart your computer if prompted.
4. After restarting, a Ubuntu terminal will open. Wait for the installation to finish and set up your Linux username and password.

### Step 2: Install Development Tools in WSL
Inside your WSL (Ubuntu) terminal, update your package lists and install `gcc` and `make`:
```bash
sudo apt update
sudo apt install build-essential
```

## Compilation

Navigate to the project directory in Windows PowerShell, and then type `wsl` to switch to your Ubuntu terminal:
```bash
wsl
```

Once in the project directory, use `make` to compile the shell:
```bash
make
```

This will generate an executable named `minishell`.

## Usage

Start the shell by running the executable:
```bash
./minishell
```

You should see the futuristic prompt with your current working directory:
`OrbitShell:/your/current/path$ `

### Available Commands
You can run any standard Unix commands available on your system (e.g., `ls`, `pwd`, `cat`, `echo`). 
Example:
```bash
OrbitShell:/...$ ls -l
OrbitShell:/...$ echo "Hello World!"
```

### Built-in Commands
The shell also includes the following built-in commands:
- **`cd [directory]`**: Changes the current working directory.
- **`env`**: Prints all environment variables.
- **`exit`**: Gracefully terminates the shell and frees all allocated memory, triggering the custom **"Breaking Orbit"** spaceship launch animation. Alternatively, you can use `Ctrl+D` (EOF) to exit.

## Cleaning Up
To remove the compiled executable, run:
```bash
make clean
```
