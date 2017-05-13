#!/bin/bash
# Copyright (C) 2012-2017 Hideaki Narita
#
# Usage: bash update_dependency.sh Makefile
#

export LANG=C

if [ $# = 0 ]; then
  echo "Usage: $0 FILENAME"
  exit 1
elif [ ! -f "$1" ]; then
  echo "$1 not found."
  exit 1
fi

lines=`wc -l "$1" | awk '{print $1}'`
line1=`grep -n '^##* dependency begin' "$1" | head -1 | awk -F: '{print $1}'`
line2=`grep -n '^##* dependency end' "$1" | head -1 | awk -F: '{print $1}'`

part1=$$.part1
part2=$$.part2
part3=$$.part3

head -$line1 "$1" > $part1
bash `dirname "$0"`/dependency.sh > $part2
tail -`expr $lines - $line2 + 1` "$1" > $part3

cat $part1 $part2 $part3 > "$1"

rm $part1
rm $part2
rm $part3
