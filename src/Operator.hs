module Operator where

import Tokenizer

neg :: Value -> Value
neg (INT x)  = INT  (- x)
neg (REAL x) = REAL (- x)

add :: Value -> Value -> Value
add (INT x)  (INT y)  = INT (x + y)
add (REAL x) (REAL y) = REAL (x + y)
add (INT x)  (REAL y) = REAL (fromIntegral x + y)
add (REAL x) (INT y)  = REAL (x + fromIntegral y)

sub :: Value -> Value -> Value
sub (INT x)  (INT y)  = INT (x - y)
sub (REAL x) (REAL y) = REAL (x - y)
sub (INT x)  (REAL y) = REAL (fromIntegral x - y)
sub (REAL x) (INT y)  = REAL (x - fromIntegral y)

mul :: Value -> Value -> Value
mul (INT x)  (INT y)  = INT (x * y)
mul (REAL x) (REAL y) = REAL (x * y)
mul (INT x)  (REAL y) = REAL (fromIntegral x * y)
mul (REAL x) (INT y)  = REAL (x * fromIntegral y)

div' :: Value -> Value -> Value
div' (INT x)  (INT y)  = INT  (x `div` y)
div' (REAL x) (REAL y) = REAL (x / y)
div' (INT x)  (REAL y) = REAL (fromIntegral x / y)
div' (REAL x) (INT y)  = REAL (x / fromIntegral y)

mod' :: Value -> Value -> Value
mod' (INT x)  (INT y)  = INT  (x `mod` y)

{-eq :: Value -> Value -> Boolean
eq x y = (x == y)

ne :: Value -> Value -> Boolean
ne x y = (x != y)

gt :: Value -> Value -> Boolean
gt x y = (x < y)

lt :: Value -> Value -> Boolean
lt x y = (x > y)

ge :: Value -> Value -> Boolean
ge x y = (x <= y)

le :: Value -> Value -> Boolean
le x y = (x >= y)

and :: Boolean -> Boolean -> Boolean
and x y = (x && y)

or :: Boolean -> Boolean -> Boolean
or x y = (x || y)

not :: Boolean -> Boolean
not x = (!x)-}