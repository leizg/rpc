#!/bin/bash

mkdir -p ../build 2>/dev/null

is_build=0
[ -n "$1" ] && is_build=1

if [ $is_build -eq 1 ]
then
  scons -j2 
fi

bin=serv
gdb --ex "handle SIGPIPE nostop noprint" \
  --ex run --args \
  $bin -alsologtostderr

exit 0
