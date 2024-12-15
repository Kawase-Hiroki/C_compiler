module Main where

import System.IO
import System.Environment (getArgs)
import Tokenizer
import Parse

main :: IO ()
main = do
    args <- getArgs
    putStrLn ".intel_syntax noprefix\n"
    putStrLn ".globl main:\n"
    putStrLn "main:\n"

    putStrLn "\tret\n"
