#!/usr/bin/env bash

# Filename:  generate.sh
# Content:   Script that generates calculator parser files
# Provided AS IS under MIT License; see LICENSE file in root folder.
#
# It generates files required by the calculator sample.
# It will create directory specified as the output for the files if it doesn't exist.

args=()
while (($#)); do
  case "$1" in
    --arch=* ) architecture="${1#*=}"; shift;;
    --arch   ) architecture="${2}"; shift 2;;

    --config=* ) configuration="${1#*=}"; shift;;
    --config  ) configuration="${2}"; shift 2;;

    --parser=* ) parser="${1#*=}"; shift;;
    --parser   ) parser="${2}"; shift 2;;

    --output-dir=* ) output_dir="${1#*=}"; shift;;
    --output-dir   ) output_dir="${2}"; shift 2;;

    * ) args+=("${1}"); shift;;
  esac
done

script_dir="$(cd $(dirname $0); pwd)"
root_dir="$(cd "${script_dir}/../.."; pwd)"
script_local_dir="${script_dir#"${root_dir}/"}"

configuration="$(echo "${configuration:-debug}" | tr '[:upper:]' '[:lower:]')"

architecture="${architecture:-x64}"

parser="${parser:-"${root_dir}/build/${architecture}/SGYacc/${configuration}/sgyacc"}"

grammar="${args[0]:-src/Calc.gr}"

output_dir="${output_dir:-generated}"
output_dir="${script_local_dir}/${output_dir}"

cd "${root_dir}"

mkdir -p "${output_dir}"

"${parser}" "${script_local_dir}/${grammar}" \
  -lalr \
  -enumclasses \
  -enumstrings \
  -dfa +f:"${output_dir}/DFA.h" +classname:"CalcDFA" \
  -prodenum +f:"${output_dir}/ProdEnum.h" \
  -pt +f:"${output_dir}/ParseTable.h" +classname:"CalcParseTable"
