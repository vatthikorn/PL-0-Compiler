# PL-0-Compiler
A PL/0 compiler with lexical analyzer, syntax analyzer and machine code generator with a PM/0 virtual machine containing features designed to support function and procedure calls with parameter passing in C.

How to compile and run it?
==========================

- Within terminal, navigate to the directory containing
the source code and the input.txt file

- Run the following command to compile:

$ gcc -o [Executable File Name] [File Name].c

Then to run the executable file:

$ ./[Executable File Name] [Command line switches]

The following command line switches print its corresponding
type of output to the screen:

-t 	The token list
-s 	The symbol table
-m 	The machine code
-a 	The disassembled code
-v 	The virtual machine execution stack trace


Example: 

$ gcc -o Compiler Compiler.c

$ ./Compiler -t -s -m -a -v

This will create seven files: cleaninput.txt, lexemetable.txt, tokenlist.txt,
symboltable.txt, mcode.txt, acode.txt and stacktrace.txt

It will also print out the token list, symbol table, machine code, disassembled code
and virtual machine execution stack trace to the screen.

The switches can be in any order, but the order of the switches does not affect the
order of output

Authors
======
Vatthikorn Apiratitham

Amanda Forster

Taylor Veith
