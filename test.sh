#!/bin/bash

rm testdate/*.png

for i in testdata/test[0-9]; do 
  echo ${i}; 
  ./a.out $i 2> /dev/null
  make plot; 
  mv output.png "${i}.png"; 
done;

