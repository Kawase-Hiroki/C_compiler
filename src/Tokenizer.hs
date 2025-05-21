module Tokenizer where

import Data.Char (isSpace, isDigit)
import Control.Monad.Writer

data Value = INT Integer | REAL Double deriving (Show, Eq)

data Token = Number Value
            | Add | Sub | Mul | Div | Mod
            {-| Eq | Ne | Gt | Lt | Ge | Le
            | And | Or | Not-}
            | Lpar | Rpar
            | Semicolon
            | Eof
            | Others Char
    deriving (Show, Eq)

type Lexer = (Token, String)

getToken :: String -> Lexer
getToken [] =(Eof, "")
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
            '%' -> (Mod, xs)
            '(' -> (Lpar, xs)
            ')' -> (Rpar, xs)
            ';' -> (Semicolon, xs)
            {-"==" -> (Eq,xs)
            "!=" -> (Ne, xs)
            '<' -> (Gt, xs)
            '>' -> (Lt, xs)
            "<=" -> (Ge, xs)
            ">=" -> (Le, xs)
            "&&" -> (And, xs)
            "||" -> (Or, xs)
            '!' -> (Not, xs)-}
            _   -> (Others x, xs)

lexer :: String -> ([Token], String)
lexer xs =
    let (t, ys) = getToken xs
    in case t of
        Semicolon -> ([Semicolon], ys)
        Eof       -> ([Eof], ys)
        _         -> let (ts,zs) = lexer ys
                     in ts `seq` zs `seq` (t:ts, zs)
