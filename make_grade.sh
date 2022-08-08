#!/bin/bash

for item in util syscall pgtbl traps cow thread fs
do
  git checkout $item
  make grade
  make clean
done
