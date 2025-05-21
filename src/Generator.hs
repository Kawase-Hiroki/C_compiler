module Generator where

import Tokenizer ( Token(Number, Lpar, Rpar, Eof, Mul, Div, Semicolon, Add, Sub), Value(REAL, INT), lexer )
import Parse ( expression, evalExpr )
import Control.Monad.Writer (runWriter)

gen :: ([Token], String) -> IO ()
gen (ys, xs) = do
    case expression ys of
        Left mes     -> putStrLn mes
        Right (e, _) -> do case evalExpr e of
                            INT x  -> print x
                            REAL x -> print x
