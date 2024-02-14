#!/bin/bash

threads=30
cpp_program="./prog"
output_file="execution_log.txt"

clang-format -i *.cpp
make
>"$output_file"

output=$("$cpp_program" $threads)
echo "$output"
echo "$output" >> $output_file
echo ""
python3 checker.py
echo ""
make clean
