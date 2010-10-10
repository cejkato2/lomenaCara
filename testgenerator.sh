#!/bin/bash

read -p "Enter filename eg. test5: " FILENAME
read -p "Enter number of points: " AMOUNT

if [ -e "testdata/$FILENAME" ]; then
  read -p "File $FILENAME will be overwritten. For cancel press CTRL+C"
fi

echo "$AMOUNT" > "testdata/$FILENAME"
for ((i=0; i<$AMOUNT; i++)); do
  echo "$((RANDOM%100)) $((RANDOM%100))" >> "testdata/$FILENAME"
done

