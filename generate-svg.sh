#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
    echo "Specify directory with .dot files"
    exit
fi

path=$(realpath $1)
svgPath="$path/svg"
mkdir -p $svgPath

for i in $(find $1 -name "*.dot" -type f); do
    filename=$(echo $i | sed 's#.*/##')
    dot $1/$filename -Tsvg -o $svgPath/$filename.svg
done
