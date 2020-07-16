#!/usr/bin/env bash

# Filename:  run.sh
# Content:   Parser smoke tests runner
# Provided AS IS under MIT License; see LICENSE file in root folder.

architecture='x64'
configuration='Debug'
grammar_dir='tests'
args=()

while (($#)); do
  case "$1" in
    --parser=* ) parser="${1#*=}"; shift;;
    --parser   ) parser="${2}"; shift 2;;

    --grammar-dir=* ) grammar_dir="${1#*=}"; shift;;
    --grammar-dir   ) grammar_dir="${2}"; shift 2;;

    --arch=* ) architecture="${1#*=}"; shift;;
    --arch   ) architecture="${2}"; shift 2;;

    --config=* ) configuration="${1#*=}"; shift;;
    --config   ) configuration="${2}"; shift 2;;

    * ) args+=("${1}"); shift;;
  esac
done

script_dir="$(cd $(dirname $0); pwd)"
root_dir="$(cd "${script_dir}/../.."; pwd)"

script_local_dir="${script_dir#"${root_dir}/"}"

cd "${root_dir}"

parser="${parser:-"${root_dir}/build/${architecture}/SGYacc/${configuration}/sgyacc"}"

grammars=()

while read -r line
do
  line="$(echo "${line%\#*}" | xargs)"
  if [ -n "${line}" ]
  then
    grammars+=(${line})
  fi
done < "${script_local_dir}/grammar.lst"

for grammar in "${grammars[@]}"
do
  grammar_path="${grammar_dir}/${grammar}"
  test_dir="${script_local_dir}/${grammar}"
  snapshot_dir="${test_dir}/Snapshot"
  output_dir="${test_dir}/Output"

  mkdir -p "${output_dir}"

  echo -n "Testing ${grammar} ..."

  output=$("${parser}" "${grammar_path}" \
    -cd +f:"${output_dir}/CanonicalData.txt" \
    -dfa +f:"${output_dir}/StaticDFA.h" \
    -nontermenum +f:"${output_dir}/NonTermEnum.h" \
    -prodenum +f:"${output_dir}/ProdEnum.h" \
    -pt +f:"${output_dir}/StaticParseTable.h" \
    -rf +f:"${output_dir}/ReduceFunction.h" \
    -termenum +f:"${output_dir}/TermEnum.h")

  if ! [ $? -eq 0 ]; then exit $?; fi

  result="$(echo "${output}" | tail -1)"
  error_count=$((${result%error*}))
  if [ ${error_count} -ne 0 ]
  then
    echo -e "\b\b\bFAIL"
    echo "${output}"
    exit 1
  fi

  output=$("${parser}" "${grammar_path}" \
    -enumclasses \
    -enumstrings \
    -nontermenum +f:"${output_dir}/NonTermEnumClsStr.h" \
    -prodenum +f:"${output_dir}/ProdEnumClsStr.h" \
    -rf +f:"${output_dir}/ReduceFunctionClsStr.h" \
    -termenum +f:"${output_dir}/TermEnumClsStr.h")

  if ! [ $? -eq 0 ]; then exit $?; fi

  result="$(echo "${output}" | tail -1)"
  error_count=$((${result%error*}))
  if [ ${error_count} -ne 0 ]
  then
    echo -e "\b\b\bFAIL"
    echo "${output}"
    exit 1
  fi

  if ! [ -d "${snapshot_dir}" ]
  then
    echo -e "\b\b\bFAIL"
    echo "Failed to locate ${snapshot_dir} directory"
    exit 1
  fi

  snapshots=($(ls "${snapshot_dir}"))

  succeeded=1

  for snapshot_file_name in "${snapshots[@]}"
  do
    expected_file="${snapshot_dir}/${snapshot_file_name}"
    actual_file="${output_dir}/${snapshot_file_name}"
    differences="$(diff "${expected_file}" "${actual_file}")"

    if [ -n "${differences}" ]
    then
      succeeded=0
      echo -e "\b\b\bFAIL"
      echo "Difference found for ${grammar} in ${snapshot_file_name}"
      echo "${differences}"
    fi
  done

  if [ ${succeeded} -eq 1 ]
  then
    rm -rf "${output_dir}"
    echo -e "\b\b\bSUCCESS"
  fi
done
