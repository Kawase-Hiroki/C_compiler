module Parse where

import Tokenizer
    ( Token(Number, Lpar, Rpar, Eof, Mul, Div, Mod, Semicolon, Add, Sub),
      Value(INT, REAL))
import Data.Char
import Distribution.Simple.Utils (xargs)
import Operator
import Control.Monad.Writer (Writer, tell)
import Operator

data Expr = Num Value
    | Op1 (Value -> Value) Expr
    | Op2 (Value -> Value -> Value) Expr Expr

type Calc a = Either String a

calcError :: String -> Calc a
calcError s = Left s

factor :: [Token] -> Calc (Expr, [Token])
factor (Number x : xs) = return (Num x, xs)
factor (Lpar : xs) = expr xs >>= \(e, y:ys) ->
                        case y of
                            Rpar -> return (e, ys)
                            _    -> calcError "')' expected"
factor (Sub : xs) = expr xs >>= \(e, ys) -> return (Op1 neg e, ys)
factor (Add : xs) = expr xs
factor (Eof : xs) = calcError "end of file"
factor (x : _)    = calcError ("unexpected token: " ++ show x)
factor []         = calcError "unexpected end of input"

term :: [Token] -> Calc (Expr, [Token])
term xs = factor xs >>= term_sub
    where
        opList = [(Mul, mul), (Div, div'), (Mod, mod')]
        term_sub :: (Expr, [Token]) -> Calc (Expr, [Token])
        term_sub (e, []) = return (e, [])
        term_sub (e, y:ys) =
            case lookup y opList of
                Nothing -> return (e, y:ys)
                Just op -> factor ys >>= \(e', ys') -> term_sub (Op2 op e e', ys')

expr :: [Token] -> Calc (Expr, [Token])
expr xs = term xs >>= expr_sub
    where
        opList = [(Add, add), (Sub, sub)]
        expr_sub :: (Expr, [Token]) -> Calc (Expr, [Token])
        expr_sub (e, []) = return (e, [])
        expr_sub (e, y:ys) =
            case lookup y opList of
                Nothing -> return (e, y:ys)
                Just op -> term ys >>= \(e', ys') -> expr_sub (Op2 op e e', ys')

expression :: [Token] -> Calc (Expr, [Token])
expression xs = expr xs >>= \(e, y:ys) ->
    case y of
        Semicolon -> return (e, ys)
        _         -> calcError "expression error"

evalExpr :: Expr -> Value
evalExpr (Num x) = x
evalExpr (Op1 op e) = op (evalExpr e)
evalExpr (Op2 op e1 e2) = op (evalExpr e1) (evalExpr e2)
