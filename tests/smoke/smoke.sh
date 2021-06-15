#!/usr/bin/env bash

# Filename:  smoke.sh
# Content:   Parser smoke tests runner
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Iterates through all grammar files from `grammar.lst`, 
# for each it runs the SGYacc executable and compares output files
# with the expected ones from the corresponding Snapshot folder.

# List of available command-line options and their default values.
# Test the x64 build by default.
architecture='x64'
# Test the Debug version by default.
configuration='Debug'
grammar_dir='tests'

# Iterate through the command-line arguments and extract parameters if any.
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

# Set script folder (where this script lives).
script_dir="$(cd $(dirname $0); pwd)"
# Set project's root folder.
root_dir="$(cd "${script_dir}/../.."; pwd)"
# Set relative path to script folder.
script_local_dir="${script_dir#"${root_dir}/"}"

cd "${root_dir}"

# Set absolute path to the configured version of the X Compiler.
parser="${parser:-"${root_dir}/build/${architecture}/SGYacc/${configuration}/sgyacc"}"

# Extract a list of grammars to test from `grammar.lst`.
grammars=()
while read -r line
do
  line="$(echo "${line%\#*}" | xargs)"
  if [ -n "${line}" ]
  then
    grammars+=(${line})
  fi
done < "${script_local_dir}/grammar.lst"

# Iterate through the list of grammar files and test them.
for grammar in "${grammars[@]}"
do
  grammar_path="${grammar_dir}/${grammar}"
  test_dir="${script_local_dir}/${grammar}"
  snapshot_dir="${test_dir}/Snapshot"
  output_dir="${test_dir}/Output"

  mkdir -p "${output_dir}"

  echo -n "Testing ${grammar} ..."

  # Run the parser and capture the output (an "old" version, without the enum classes).
  output=$("${parser}" "${grammar_path}" \
    -cd +f:"${output_dir}/CanonicalData.txt" \
    -dfa +f:"${output_dir}/StaticDFA.h" \
    -nontermenum +f:"${output_dir}/NonTermEnum.h" \
    -prodenum +f:"${output_dir}/ProdEnum.h" \
    -pt +f:"${output_dir}/StaticParseTable.h" \
    -rf +f:"${output_dir}/ReduceFunction.h" \
    -termenum +f:"${output_dir}/TermEnum.h")

  # Exit if no output.
  if ! [ $? -eq 0 ]; then exit $?; fi

  # Get the number of errors from the output, and exit if any.
  result="$(echo "${output}" | tail -1)"
  error_count=$((${result%error*}))
  if [ ${error_count} -ne 0 ]
  then
    echo -e "\b\b\bFAIL"
    echo "${output}"
    exit 1
  fi

  # Run the parser and capture the output (a "new" version, with the enum classes).
  output=$("${parser}" "${grammar_path}" \
    -namespaces +nsname:XC \
    -enumclasses \
    -enumstrings \
    -nontermenum +f:"${output_dir}/NonTermEnumClsStr.h" \
    -prodenum +f:"${output_dir}/ProdEnumClsStr.h" \
    -rf +f:"${output_dir}/ReduceFunctionClsStr.h" \
    -termenum +f:"${output_dir}/TermEnumClsStr.h")

  # Exit if no output.
  if ! [ $? -eq 0 ]; then exit $?; fi

  # Get the number of errors from the output, and exit if any.
  result="$(echo "${output}" | tail -1)"
  error_count=$((${result%error*}))
  if [ ${error_count} -ne 0 ]
  then
    echo -e "\b\b\bFAIL"
    echo "${output}"
    exit 1
  fi

  # Check and exit of no snapshot for the current grammar file are available.
  if ! [ -d "${snapshot_dir}" ]
  then
    echo -e "\b\b\bFAIL"
    echo "Failed to locate ${snapshot_dir} directory"
    exit 1
  fi

  snapshots=($(ls "${snapshot_dir}"))

  succeeded=1

  # Check all generated files for equality with snapshot files.
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

  # Log the success if needed.
  if [ ${succeeded} -eq 1 ]
  then
    rm -rf "${output_dir}"
    echo -e "\b\b\bSUCCESS"
  fi
done
