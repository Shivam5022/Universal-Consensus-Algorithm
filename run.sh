#!/bin/bash

cpp_program="./prog"
output_file="execution_log.txt"

threads=20

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
