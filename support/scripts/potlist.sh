#!/bin/bash
# Copyright (C) 2012-2017 Hideaki Narita

LANG=C ; export LANG

echo 'POTFILES=\'
for f in `find . -name '*.cc' -exec fgrep -q gettext {} \; -print | sort` ; do
  g=`echo $f | sed 's@^\./@@'`
  echo "\$(OBJDIR)/${g/.cc/.pot} \\"
done
echo ""
