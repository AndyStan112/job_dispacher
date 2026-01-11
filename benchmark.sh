#!/usr/bin/env bash

INPUT="./in/benchmark.txt"
OUT="./data/local.csv"
BIN="dispatcher.o"


mpicc src/*.c -Iinclude -o "$BIN" || exit 1

echo "workers,time" > "$OUT"

for n in $(seq 2 11); do
    workers=$((n - 1))
    output=$(mpirun -n "$n" ./"$BIN" "$INPUT")
    time=$(echo "$output" | grep -oE '[0-9]+\.[0-9]+')
    echo "$workers,$time" >> "$OUT"
done
