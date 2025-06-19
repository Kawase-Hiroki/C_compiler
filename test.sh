#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./main "$input" > foo.s
    gcc -o foo foo.s
    ./foo
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 8 "int main () {int a; int b; a = 3; b = 5; return (a + b);}"
assert 6 "int main () {int foo; foo = 1; int bar; bar = 2 + 3; foo + bar;}"
assert 14 "int main () {int a; a = 3; int b; b = 5 * 6 - 8; return a + b / 2;}" 
assert 55 "int main () {int sum; sum = 0; int i; for (i = 0; i < 10; i = i + 1) {sum = sum + i;} return sum;}"
assert 1 "int main () {int n; n = 10; if (n == 10) {return 1;}}"
assert 0 "int main () {int n; n = 10; if (n != 10) {return 1;} else {return 0;}}"
assert 10 "int main () {int i; i = 0; while (i < 10) {i = i + 1;} return i;}"
assert 1 "int foo() {return 1;} int main() {return foo();}"
assert 2 "int foo(int x) {return x + 1;} int main() {int x; x = 1; return foo(x);}"
assert 3 "int foo(int x, int y) {return x + y;} int main() {int x; x = 1; int y; y = 2; return foo(x, y);}"
assert 3 "int main () {x = 3; y = &x; return *y;}"

echo OK