#!/bin/bash
for f in ./test/lab2/sample*.txt; do
    ./Main < "$f"
done