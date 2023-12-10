#!/bin/bash

set -e

if [ -e /proc/cpuinfo ]; then
  egrep -q 'BCM2708|BCM2711' /proc/cpuinfo
  if [ $? = 0 ]; then
    echo "=== ENABLE RASPBERRY PI ==="
    extra_flags="-DRASPBERRY_PI $extra_flags"
  fi
fi

cmd=${1:-dist}

if [ "$cmd" == "clean" ]; then
  cmd=""
fi

make -f Makefile.linux clean $cmd USER="$extra_flags"
