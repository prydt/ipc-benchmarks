#!/bin/bash

# Find files in the current directory
files=$(find . -type f -mtime -14 -printf '%f\n')

# Loop through each file
for file in $files; do
    # Check if the file name contains a period
    if [[ ! "$file" =~ \. ]]; then
        echo "working on ${file}"
        ./generate_flamegraph.sh "./${file}" $file
    fi
done
