#!/bin/bash
# run this script to create all the autotools fluff.

set -e

CURDIR=$(pwd)
SCRIPT_DIR=$(cd $(dirname $0); pwd)

cd $SCRIPT_DIR

[[ ! -d m4 ]] && mkdir m4

intltoolize -f
autoreconf -v -i -f
