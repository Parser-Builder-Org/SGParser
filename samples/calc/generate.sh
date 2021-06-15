#!/usr/bin/env bash

# Filename:  generate.sh
# Content:   Bash script that generates Calc parser source files.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Generate parser source files required by the Calc sample.
# Will create a folder specified as the output for the files if it doesn't exist.
# This script should be run after some changes were made in the Calc grammar.
# Only x64 architecture is supported.
# Examples of usage:
# - generate by default version of SGYacc:  ./generate.sh
# - generate by debug version of SGYacc:    ./generate.sh --config Debug
# - generate by debug version of SGYacc:    ./generate.sh --config Release

# List of available command-line options and their default values.
# Generate by x64 version of SGYacc by default.
# Currently, only x64 architecture is supported.
architecture='x64'
# Generate by debug version of SGYacc by default.
configuration='Debug'
# Default folder for generated files.
output_dir="${output_dir:-generated}"

# Iterate through the command-line arguments and extract parameters if any.
args=()
while (($#)); do
  case "$1" in
    --arch=* ) architecture="${1#*=}"; shift;;
    --arch   ) architecture="${2}"; shift 2;;

    --config=* ) configuration="${1#*=}"; shift;;
    --config  ) configuration="${2}"; shift 2;;

    --output-dir=* ) output_dir="${1#*=}"; shift;;
    --output-dir   ) output_dir="${2}"; shift 2;;

    * ) args+=("${1}"); shift;;
  esac
done

# Get script root folder.
script_dir="$(cd $(dirname $0); pwd)"
# Set relative path to script.
root_dir="$(cd "${script_dir}/../.."; pwd)"
script_local_dir="${script_dir#"${root_dir}/"}"

# Set parameters-dependent path to SGYacc executable.
parser="${parser:-"${root_dir}/build/${architecture}/SGYacc/${configuration}/sgyacc"}"

# Set Calc gramma file.
grammar="${args[0]:-src/Calc.gr}"

output_dir="${script_local_dir}/${output_dir}"

cd "${root_dir}"

# Create output folder if needed.
mkdir -p "${output_dir}"

# Generate Clac parser source files with all configired options.
"${parser}" "${script_local_dir}/${grammar}" \
  -lalr \
  -enumclasses \
  -enumstrings \
  -namespaces +nsname:Calc \
  -dfa +f:"${output_dir}/DFA.h" +classname:"CalcDFA" \
  -prodenum +f:"${output_dir}/ProdEnum.h" \
  -pt +f:"${output_dir}/ParseTable.h" +classname:"CalcParseTable"
