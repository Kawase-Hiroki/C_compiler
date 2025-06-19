# C-_compiler

I try to make C language compiler.

## How to perform compile

```
cd /home/hiroki/C_compiler
gcc -g src/main.c src/tokenizer.c src/generator.c src/parse.c src/error.c -o main
./main "int foo(int x) {return x + 1;} int main() {int x; x = 1; return foo(x);}" > foo.s
gcc -o foo foo.s
./foo
echo $?
```

chmod a+x test.sh
./test.sh