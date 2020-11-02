#!/usr/bin/env bash

if [ "$#" -ne 1 ];
    then echo "Specify .dot file path as first parameter"
    exit
fi

dot -Tsvg -O $1
