# C-_compiler

I try to make C language compiler.

## How to perform compile

```
gcc -g main.c tokenizer.c generator.c parse.c error.c -o main
./main "12 + 34 - 5" > foo.s
gcc -o foo foo.s
./foo
echo $?
```
