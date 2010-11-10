#!/bin/bash
set -x
mkdir -p testdata/png
rm -f testdate/png/*.png

for i in testdata/test*; do 
  echo ${i}; 
  ./a.out $i 2> /dev/null
  make plot > /dev/null; 
  mv output.png "testdata/png/$(basename ${i}).png"; 
done;

