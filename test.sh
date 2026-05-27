#!/bin/bash
for f in ./test/sample*.txt; do
    ./Main < "$f"
done