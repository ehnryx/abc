#!/bin/bash

scripts_dir=$(dirname "$(readlink -f "$0")")
ABC_ROOT=$(realpath "${scripts_dir}/../")
export ABC_ROOT
echo "${ABC_ROOT}"
