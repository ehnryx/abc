#!/bin/bash

start=${EPOCHREALTIME}
"./${1}" < "${2}" > /dev/null
end=${EPOCHREALTIME}

awk '{printf "%f\n", $1 - $2}' <<< "${end} ${start}"
