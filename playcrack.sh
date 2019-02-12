#!/bin/bash

##------------------------------------------------------------------------|
## Copyright (c) 2019 by Raymond M. Foulk IV
##
## Permission is hereby granted, free of charge, to any person obtaining a
## copy of this software and associated documentation files (the
## "Software"), to deal in the Software without restriction, including
## without limitation the rights to use, copy, modify, merge, publish,
## distribute, sublicense, and/or sell copies of the Software, and to
## permit persons to whom the Software is furnished to do so, subject to
## the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
## IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
## CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
## TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
## SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
##------------------------------------------------------------------------|

##------------------------------------------------------------------------|
# Purpose: This script will read in a word dictionary file, expected to be
# a text file with one word per line.  It will then brute force try every
# possible word combination as playfair passphrases, from 1 up to 3 words
# combined, for a total of N + N^2 + N^3 iterations.  It will use these
# against the provided ciphertext, and grep the output for the provided
# regular expression, filtering out everything that doesn't match.

dictionary[0]=""
dictEntries=0

##------------------------------------------------------------------------|
function playcrack_load_dictionary
{
    # Check for existance of dictionary file
    if [ ! -e "$1" ]
    then
        echo "ERROR: Dictionary file $1 does not exist"
        exit 1
    fi

    # Get number of lines
    dictEntries=`cat "$1" | wc -l`
    if [ $dictEntries -eq 0 ]
    then
        echo "ERROR: Dictionary file $1 contains no entries"
        exit 2
    fi

    # Read in dictionary file to array
    dictEntry=1
    while [ $dictEntry -le $dictEntries ]
    do
        # Get the word
        dictionary[$dictEntry]=`sed -n "$dictEntry p" "$1"`
        #echo "dictWord: ${dictionary[$dictEntry]}"
        dictEntry=$[$dictEntry+1]
    done
}

##------------------------------------------------------------------------|
function playcrack_try_passphrase
{
    echo "------------ trial -------------"
    echo "passphrase: $1"
    message=`./playfair -p "$1" -d "$2"`
    echo "message: $message"
    echo
}

##------------------------------------------------------------------------|
function playcrack_iterate_combos
{
    word1=0
    while [ $word1 -le $dictEntries ]
    do
        word2=0
        while [ $word2 -le $dictEntries ]
        do
            word3=0
            while [ $word3 -le $dictEntries ]
            do
                playcrack_try_passphrase "${dictionary[$word1]} ${dictionary[$word2]} ${dictionary[$word3]}" "$1"
                word3=$[$word3+1]
            done
            word2=$[$word2+1]
        done
        word1=$[$word1+1]
    done
}

##------------------------------------------------------------------------|
function playcrack_main
{
    # show help if not enough arguments
    if [ -z "$2" ]
    then
        echo "usage: ./playcrack.sh <dictionary-file> <ciphertext>"
        echo
        echo "The ./playfair executable is expected to be in the path"
        echo
        exit 1
    fi

    playcrack_load_dictionary "$1"
    playcrack_iterate_combos "$2"
}

##------------------------------------------------------------------------|
playcrack_main $*
