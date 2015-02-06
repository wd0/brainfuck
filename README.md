# brainfuck
Brainfuck interpreter. 
Reads whole files from the command line and executes them sequentially, or read all of standard input and then begin executing.

## Build
Run make.

## Notes
The stack is fixed at 30,000 bytes. An access outside that region terminates the execution of that particular brainfuck subprogram.
