#!/bin/sh
# run this script to create all the autotools fluff.

set -e

CURDIR=$(pwd)
SCRIPT_DIR=$(cd $(dirname $0); pwd)

cd $SCRIPT_DIR

if [ ! -d m4 ]; then
  mkdir m4
fi

intltoolize -f
autoreconf -v -i -f
