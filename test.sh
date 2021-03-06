#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 '0;'
assert 42 "42;"
assert 25 "5+20;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 5 '+5;'
assert 10 '-10+20;'
assert 1 '2 == 2;'
assert 0 '12 == 1;'
assert 1 '100 != 1;'
assert 0 '100 != 2 * 50;'
assert 1 '1 < 2;'
assert 0 '1 < 1;'
assert 0 '1 > 1;'
assert 0 '2 <= 1;'
assert 1 '1 <= 1;'
assert 1 '1 >= 1;'
assert 0 '1 >= 5;'
assert 0 '4+6*7 < 46;'
assert 1 '4+6*7 <= 46;'
assert 52 'a = 52;a;'
assert 1 "a = 1;
a;
"
assert 14 "a = 3;
b = 5 * 6 - 8;
a + b / 2;"


echo OK
