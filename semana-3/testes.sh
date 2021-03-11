#!/bin/bash
for x in externo.o interno.o intermediario.o; do
    echo "ARQUIVO $x"
    for y in 1 2 4 8 16 32 64 128; do
        echo "NUM_ THREADS = $y"
        for i in 512 1024 2048; do
            echo "$i -"
            echo "$i" "$i" "$i" "$y"| OMP_NUM_THREADS=$y ./$x
        done
    done
done


