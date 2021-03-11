#!/bin/bash

for file in externo intermediario interno; do
    gcc $file.c -o $file.o -fopenmp
done