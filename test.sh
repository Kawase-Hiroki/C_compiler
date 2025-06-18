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

assert 8 "main () {a = 3; b = 5; return (a + b);}"
assert 6 "main () {foo = 1; bar = 2 + 3; foo + bar;}"
assert 14 "main () {a = 3; b = 5 * 6 - 8; return a + b / 2;}" 
assert 55 "main () {sum = 0; for (i = 0; i < 10; i = i + 1) {sum = sum + i;} return sum;}"
assert 1 "main () {n = 10; if (n == 10) {return 1;}}"
assert 0 "main () {n = 10; if (n != 10) {return 1;} else {return 0;}}"
assert 10 "main () {i = 0; while (i < 10) {i = i + 1;} return i;}"
assert 1 "foo() {return 1;} main() {return foo();}"
assert 2 "foo(x) {return x + 1;} main() {x = 1; return foo(x);}"
assert 3 "foo(x, y) {return x + y;} main() {x = 1; y = 2; return foo(x, y);}"
assert 3 "main () {x = 3; y = &x; return *y;}"

echo OK