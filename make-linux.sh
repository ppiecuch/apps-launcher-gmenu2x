#!/bin/bash

if [ -x /proc/cpuinfo ]; then
  grep -q BCM2708 /proc/cpuinfo
  if [ $? = 0 ]; then
    extra_flags="-DRASPBERRY_PI $extra_flags"
  fi
fi

make -f Makefile.linux dist USER="$extra_flags"
