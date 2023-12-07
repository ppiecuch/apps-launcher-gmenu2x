#!/bin/bash

set -e

rm -f gmenu2x-dist.tgz
tar czf gmenu2x-dist.tgz -C dist/linux .
scp gmenu2x-dist.tgz 192.168.1.60:
