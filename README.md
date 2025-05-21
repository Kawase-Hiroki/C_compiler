# C-_compiler

I try to make C language compiler.

## How to perform compile

```
gcc -g main.c tokenizer.c generator.c parse.c error.c -o main
./main "a = 3; b = 5; a+b;" > foo.s
gcc -o foo foo.s
./foo
echo $?
```
