#!/bin/bash
# Copyright (C) 2012-2017 Hideaki Narita

LANG=C ; export LANG

tmp1=$$.tmp1
tmp2=$$.tmp2

list_files() {
  grep '#include[ \t]*".*"' "$1" | sed 's@^[^"]*"\([^"]*\)".*$@\1@'
}

add_file() {
  fgrep -q "$1" $tmp1 $tmp2 || echo "$1" >> $tmp2
}

extract3() {
  if [ -f "$1" ]; then
    add_file "$1"
  elif [ -f "$2/$1" ]; then
    add_file "$2/$1"
  else
    d=`dirname "$1"`
    b=`basename "$1"`
    if [ "$d" = "." ]; then
      f=`find . -name "$b" -print | head -1 | sed 's@^\./@@'`
      if [ ! -z "$f" ]; then
        add_file "$f"
      fi
    else
      echo "NOT FOUND: $1" >&2
    fi
  fi
}

extract2() {
  cp /dev/null $tmp2
  d=`dirname "$1"`
  for f in `list_files "$1"` ; do
    extract3 "$f" "$d"
  done
  if [ -s $tmp2 ]; then
    cat $tmp2 >> $tmp1
    for f in `cat $tmp2` ; do
      extract2 "$f"
    done
  fi
}

extract1() {
  cp /dev/null $tmp1
  extract2 "$1"
  if [ -s $tmp1 ]; then
    echo "\$(OBJDIR)/${1/.cc/.o}: $1 `sort $tmp1 | tr '\n' ' '`"
  fi
}

for f in `find . -name '*.cc' -print | sed 's@^\./@@' | sort` ; do
  extract1 "$f"
done

rm $tmp1
rm $tmp2
