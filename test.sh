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

echo OK