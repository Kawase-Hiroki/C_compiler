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

echo OK