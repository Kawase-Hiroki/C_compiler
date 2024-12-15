module C_compiler.Tokenizer where

import Data.Char (isSpace, isDigit)

data Value = INT Integer | REAL Double deriving (Show, Eq)

--define token data
data Token = Number Value
            | Add | Sub | Mul | Div
            | Lpar | Rpar
            | Semicolon
            | E0F
            | Others Char
    deriving (Show, Eq)

type Lexer = (Token, String)

--get token by type
getToken :: String -> Lexer
getToken [] =(E0F, "")
getToken (x:xs)
    | isSpace x = getToken xs
    | isDigit x =
        let (s, ys) = span isDigit (x:xs)
        in case ys of
            (y:ys') | y == '.' || y == 'e' || y == 'E' ->
                case reads (x:xs) of
                    [] -> error "not a number"
                    ((y', ys''):_) -> (Number (REAL y'), ys'')
            _ -> (Number (INT (read s)), ys)
    | otherwise =
        case x of
            '+' -> (Add, xs)
            '-' -> (Sub, xs)
            '*' -> (Mul, xs)
            '/' -> (Div, xs)
            '(' -> (Lpar, xs)
            ')' -> (Rpar, xs)
            ';' -> (Semicolon, xs)
            _   -> (Others x, xs)

--generate token string
lexer :: String -> ([Token], String)
lexer xs =
    let (t, ys) = getToken xs
    in case t of
        Semicolon -> ([Semicolon], ys)
        E0F       -> ([E0F], ys)
        _         -> let (ts,zs) = lexer ys
                     in ts `seq` zs `seq` (t:ts, zs)