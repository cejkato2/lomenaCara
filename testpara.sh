#!/bin/bash
set -x
mkdir -p testdata/pngPara
rm -f testdate/pngPara/*.png

for i in testdata/test*; do 
  echo ${i}; 
  mpirun -np 3 ./a.out $i 2>/dev/null
  make plot > /dev/null; 
  mv output.png "testdata/pngPara/$(basename ${i}).png"; 
done;

