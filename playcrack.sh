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
}

##-----------------------------------------------------------------------##
function playcrack_next {
  # check for existance of input database file

    # check for empty database
    if [ $range_entries -eq 0 ]
    then
      echo "error: address database is empty"
    else
      # generate a random line number
      range_random=$RANDOM
      #let "range_random %= $range_entries"
      range_random=$[$range_random%$range_entries]
      range_random=$[$range_random+1]

      # get and remove the address at the specified line
      address=`sed -n "$range_random p" "$range_db_in"`

      # report address and record to output database
      echo "$address"
    fi
  fi
}

##-----------------------------------------------------------------------##
function playcrack_main {
  # show help if not enough arguments
  if [ -z "$3" ]
  then
      echo "usage: ./playcrack.sh <dictionary-file> <ciphertext> <expect-regex>"
      echo
      echo "The ./playfair executable is expected to be in the path"
      echo
      exit 1
  fi



}

##-----------------------------------------------------------------------##
playcrack_main $*
