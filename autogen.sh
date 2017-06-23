#!/bin/bash
# run this script to create all the autotools fluff.

set -e

CURDIR=$(pwd)
SCRIPT_DIR=$(cd $(dirname $0); pwd)

cd $SCRIPT_DIR

intltoolize
autoreconf -v -i -f
