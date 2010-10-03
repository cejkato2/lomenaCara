#!/bin/bash

for i in test/test*; do 
  echo ${i}; 
  ./a.out ${i} 2> /dev/null; 
  make plot; 
  mv output.png ${i}.png; 
done;

