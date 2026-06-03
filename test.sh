#!/bin/bash
for f in ./test/lab3/sample*.txt; do
    ./Main < "$f"
done