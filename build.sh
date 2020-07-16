#!/usr/bin/env bash

# Filename:  build.sh
# Content:   Bash script for building CMake-based version of SGYacc
# Provided AS IS under MIT License; see LICENSE file in root folder.

architecture='x64'
configuration='Debug'
cmake='cmake'
args=()

while (($#)); do
  case "$1" in
    --arch=* ) architecture="${1#*=}"; shift;;
    --arch   ) architecture="${2}"; shift 2;;

    --config=* ) configuration="${1#*=}"; shift;;
    --config   ) configuration="${2}"; shift 2;;

    --cmake=* ) cmake="${1#*=}"; shift;;
    --cmake   ) cmake="${2}"; shift 2;;

    * ) args+=("${1}"); shift;;
  esac
done

if [ "$(echo "${configuration}" | tr '[:upper:]' '[:lower:]')" = "debug" ]
then
  parallel=1
else
  parallel=4
fi

script_dir="$(cd $(dirname $0); pwd)"
root_dir="${script_dir}"

project_dir="${root_dir}"
cmake_output_dir="${root_dir}/build/${architecture}"

"${cmake}" -S "${project_dir}" -B "${cmake_output_dir}" -DCMAKE_BUILD_TYPE="${configuration}"

"${cmake}" --build "${cmake_output_dir}" --config "${configuration}" --parallel "${parallel}"
