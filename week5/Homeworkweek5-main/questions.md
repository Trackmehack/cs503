Muhammad Farooq
02 -06-2025
QUESTIONS


1. fgets() works well because it reads one line at a time, prevents buffer overflow, and handles EOF properly.

2. We use malloc() for cmd_buff instead of a fixed-size array because it lets us allocate memory as needed, saves space, and avoids wasting memory on unused space.

3. Trimming spaces in build_cmd_list() is necessary to prevent errors, make sure commands are recognized correctly, and avoid unexpected issues when parsing input.

4. Examples of redirection and challenges:

    ls > output.txt sends output to a file (challenge: avoid overwriting existing data).
    ls 2> error.txt saves errors to a file (challenge: handle file permissions properly).
    ls > output.txt 2>&1 combines output and errors (challenge: keep them in the right order).

5. Redirection and piping are different because redirection (>, <) moves output to a file, while piping (|) sends it to another command to process further.

6. STDERR should stay separate from STDOUT so errors don’t get mixed with normal output, making debugging easier and keeping logs clean.

7. The shell should handle errors by showing error messages separately but allowing users to combine them with 2>&1 when needed.