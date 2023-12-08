#!/bin/bash

set -e

rm -f gmenu2x-dist.tgz
echo "== Packing dist .."
tar czf gmenu2x-dist.tgz -C dist/linux .
echo "== Sending to console .."
scp gmenu2x-dist.tgz 192.168.1.60:
