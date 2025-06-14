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

assert 8 "a = 3; b = 5; a + b;"
assert 6 "foo = 1; bar = 2 + 3; foo + bar;"
assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;" 
assert 55 "sum = 0; for (i = 0; i < 10; i = i + 1) {sum = sum + i;} return sum;"
assert 1 "n = 10; if (n == 10) {return 1;}"
assert 0 "n = 10; if (n != 10) {return 1;} else {return 0;}"
assert 10 "i = 0; while (i < 10) {i = i + 1;} return i;"

echo OK