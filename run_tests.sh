#!/bin/bash 

# Make the target directory if it doesn't exist

build_path="target/test_bin"

if [[ ! -d "target" ]]; then
  mkdir target
  mkdir target/test_bin
fi 

if [[ ! -d "target/test_bin" ]]; then
  mkdir target/test_bin
fi

make test

for file in $build_path/*; do

  echo "Running test: $file"
  valgrind $file 2>&1 | grep -EA 5 "ERROR SUMMARY|LEAK SUMMARY" 
  echo "============================================================"
done

# make the tests 

