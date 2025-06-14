# C-_compiler

I try to make C language compiler.

## How to perform compile

```
cd /home/hiroki/C_compiler
gcc -g src/main.c src/tokenizer.c src/generator.c src/parse.c src/error.c -o main
./main "n = 10; if (n == 10) {return 1;} else {return 0;}" > foo.s
gcc -o foo foo.s
./foo
echo $?
```

chmod a+x test.sh
./test.sh