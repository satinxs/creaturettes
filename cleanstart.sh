#!/usr/bin/env bash

set -x

rm ./*.exe*
rm ./*.pdb

rm ./nob ./main

cc -o nob src/nob.c
