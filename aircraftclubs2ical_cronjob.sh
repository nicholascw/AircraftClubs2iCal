#!/bin/bash
DIR="$(dirname "${BASH_SOURCE[0]}")"
DIR="$(realpath "${DIR}")"

## $1 user; $2 pass; $3 output file; $4 tzid; $5 google calendar name;

$DIR/ac2ical -u $1 -p $2 -o $3 -t $4 > /dev/null
yes | gcalcli --nocache --calendar $5 delete \*
gcalcli --nocache --calendar $5 import $3
