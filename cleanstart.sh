#!/usr/bin/env bash

set -x

rm ./*.exe*
rm ./*.pdb

cc -o nob nob.c
