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


# read dictionary file
# try word combos
# grep for heywords

##------------------------------------------------------------------------|
function playcrack_usage {
  echo "usage: ./playcrack.sh <dictionary-file>"
  echo
}

##-----------------------------------------------------------------------##
function playcrack_coerce {
  # set default range if no arguments
  if [ -z $1 ]
  then
    echo "warning: playcrack_coerce: no range coerce argument"
    range_coerce=0
  else
    range_coerce=$1
  fi

  # coerce the value to within acceptible bounds
  if [ $range_coerce -lt $range_min ]
  then
    range_coerce=$range_min
  elif [ $range_coerce -gt $range_max ]
  then
    range_coerce=$range_max
  fi
}

##-----------------------------------------------------------------------##
function playcrack_next {
  # check for existance of input database file
  if [ ! -e "$range_db_in" ]
  then
    echo "error: $range_db_in does not exist"
  else
    # get number of lines in database file
    range_entries=`wc -l "$range_db_in" | sed -e 's/^[ ^t]*//g' | \
      cut --delimiter=' ' --fields=1`

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
      sed "$range_random d" "$range_db_in" > "$range_db_tmp"
      mv -f "$range_db_tmp" "$range_db_in"
      # note - this is disk i/o intensive!! :(

      # report address and record to output database
      echo "$address"
      echo "$address" >> "$range_db_out"
      #echo "range_entries: $range_entries"
      #echo "range_random: $range_random"
    fi
  fi
}

##-----------------------------------------------------------------------##
function playcrack_main {
  # show help if no mode argument
  if [ -z "$1" ]
  then
    playcrack_usage


  elif [ "$1" == "reset" ]
  then
    playcrack_reset

  else
    echo "error: unknown mode: $1"
  fi
}

##-----------------------------------------------------------------------##
playcrack_main $*


