# math-c
A simple tool to experiment with mathematical expression parsing.
You can create variables, use built-in (or define your own) functions that perform
calculations.

# Guide
## Expression
Evalueate a simple expression
```c
>>> 1 + 2
= 3.00000
```
Evaluate an expression with functions.
Functions can have only one parameter that can be a number or an expression.
`function[expression]`
```c
>>> 1 + sq[8]
= 65.00000
```
You can use multiple functions inside of each other
```c
>>> sq[8 + sq[2]]
= 144.00000
```
Evaluate an expression with variables.
```c
>>> x + 1
= ...
```

## Function definition
To define a function, use the following syntax
`#name (const) {expression}`
OR
`#name {expression}`
Constant functions can't be modified anymore after definition.
The `?` stands for the fuunction parameter.
```c
>>> #increment {? + 1}
>>> increment[5]
= 6.00000
```
# Variable declaration
To declare a variable, use the following syntax
`@name (const) = expression`
OR
`@name = expression`
Constant variables can't be modified anymore after declaration.
```c
>>> @xyz = 5
>>> xyz
= 5.00000
```

## Compiling on Linux
### Compiling on Linux with GCC
To compile this program, download the main.c file and use the following command.
```sh
gcc -o main.o main.c -lm
```
