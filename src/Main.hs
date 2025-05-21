module Main where

import System.IO
import System.Environment (getArgs)
import Tokenizer (lexer)
import Parse ()
import Generator ( gen )
import Data.Char(toUpper)

main :: IO ()
main = do
    xs <- getLine
    let (ys, xs') = lexer xs
    gen (ys, xs')
