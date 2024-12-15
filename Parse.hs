module C_compiler.Parse where

import Tokenizer
import Data.Char

data Expr = Num Value
    | Op1 (Value -> Value) Expr
    | Op2 (Value -> Value -> Value) Expr Expr

type Calc a = Either String a

calcError :: String -> Calc a
calcError = Left

factor :: [Token] -> Calc (Expr, [Token])
factor (Number x : xs) = return (Num x, xs)
factor (Lpar : xs) = expr xs >>= \(e, y:ys) ->
    case y of
        Rpar -> return (e,ys)
        _    -> calcError "')' expected"
factor (Sub : xs) = expr xs >>= \(e, ys) -> return (Op1 neg e, ys)
factor (Add : xs) = expr xs
factor (E0F : es) = calcError "end of file"
factor (x :_)     = calcError ("unexpected token: " ++ show x)

term :: [Token] -> Calc (Expr, [Token])
term xs = factor xs >>= term_sub
    where
        opList = [(Mul, mul), (Div, div')]
        term_sub zs@(e, y:ys) =
            case lookup y opList of
                Nothing -> return zs
                Just op -> factor ys >>= \(e', ys') -> term_sub (Op2 op e e', ys')

expr :: [Token] -> Calc (Expr, [Token])
expr xs = factor xs >>= term_sub
    where
        opList = [(Add, add), (Sub, sub)]
        term_sub zs@(e, y:ys) =
            case lookup y opList of
                Nothing -> return zs
                Just op -> factor ys >>= \(e', ys') -> term_sub (Op2 op e e', ys')

expression :: [Token] -> Calc (Expr, [Token])
expression xs = expr xs >>= \(e, y:ys) ->
    case y of
        Semicolon -> return (e, ys)
        _         -> calcError "expression error"