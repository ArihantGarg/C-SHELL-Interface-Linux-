# Command Line Interface

This is a simple command-line interface (CLI) implemented in C that supports basic shell functionalities. It includes features such as executing commands, file input/output redirection, changing directories, and maintaining a command history.

## Features
- **Command Execution:** Execute various commands entered by the user.
- **File Input/Output Redirection:** Redirect input from and output to files using `<` and `>` operators.
- **Change Directory (cd):** Change the current working directory.
- **Command History:** Maintain a history of the last 10 commands entered.
- **Piping:** Execute commands with pipe operators (`|`) for inter-process communication.

## How to Use
1. **Compilation:**
   - Compile the program using a C compiler. For example:
     ```bash
     gcc ShellInterface.c -o ShellInterface
     ```
2. **Run the Program:**
   - Execute the compiled program:
     ```bash
     ./ShellInterface
     ```
3. **Enter Commands:**
   - Enter commands in the CLI, and press Enter to execute.
   - Use `<` and `>` for file input/output redirection.
   - Use `cd` to change the working directory.
   - Use `exit` or `kill` to exit the CLI.

## Examples
- Execute a command: 
  ```bash
  ls -l

## How It Works

The CLI program follows a modular structure and includes the following key components:

- **Command Parsing:** The program parses user commands, identifies built-in commands, and separates arguments.

- **File I/O Redirection:** It handles file input (`<`) and output (`>`) redirection, allowing users to read from or write to files.

- **Built-in Commands:** The CLI supports built-in commands like `pwd`, `cd`, `exit` (or `kill`), and `history`.

- **Command Execution:** It executes both built-in and non-built-in commands. Non-built-in commands are executed in child processes.

- **Piping:** The program supports commands connected by pipes (`|`) for inter-process communication.

## Usage Tips

- **File Input/Output:** Utilize `<` and `>` for redirecting input from or output to files, respectively.

- **Changing Directory:** Use `cd` followed by the desired path to change the working directory.

- **Command History:** Retrieve the command history using the `history` command.

- **Piping Commands:** Combine commands using the `|` operator for piping. For example:
  ```bash
  ls -l | grep txt
