#!/bin/sh

filesdir=$1
searchstr=$2

if [ -z "$1" ] || [ -z "$2"   ]
then
    echo "Error: One of the required parameters is not declared."
    exit 1
fi
if [ ! -d "$filesdir" ]
then
    echo "Error: $filesdir does not represent a directory on the filesystem."
    exit 1
fi


num_files=$(find "$filesdir" -type f | wc -l) # Find and count the number of unique files in the directory and subdirectories


num_matching_lines=$(grep -r "$searchstr" "$filesdir" | wc -l) # Count the number of lines containing the search string

echo "The number of files are $num_files and the number of matching lines are $num_matching_lines."
