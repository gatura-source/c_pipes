# c_pipes
Learning pipes using a birthday calculator
# c_pipes
Learning pipes using a birthday calculator

## usp.c

`usp.c` demonstrates interprocess communication (IPC) in C using pipes and forked processes. It scans the current directory for files with a `.usp` extension, each containing a name and a date of birth (in `DD-MM-YYYY` format). For each file:

1. The parent process forks a child.
2. The parent sends the filename to the child via a pipe.
3. The child reads the file, extracts the name and date of birth, calculates the age, and sends the result back to the parent through another pipe.
4. The parent writes the result (in the format `Name:Age`) to `result.txt`.

### Usage

1. Place `.usp` files in the same directory, each containing:
    ```
    Name
    DD-MM-YYYY
    ```
2. Compile:
    ```
    gcc [usp.c](http://_vscodecontentref_/0) -o usp
    ```
3. Run:
    ```
    ./usp
    ```
4. Results will be appended to `result.txt`.

### Features

- Demonstrates use of `fork()`, `pipe()`, and basic file I/O.
- Calculates age based on the current year.
- Handles multiple `.usp` files in the directory.

### References

See comments in `usp.c` for detailed references and learning resources.