#!/usr/bin/env bash

# Filename:  build.sh
# Content:   Bash script for building CMake-based version of SGYacc.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Build Parser and Parser Generator libraries and then build SGYacc executable 
# and Sample executables according to specified configuration.
# Only x64 architecture is supported.
# Examples of usage:
# - build with default parameters:  ./build.sh
# - build debug version:            ./build.sh --config debug
# - build release version:          ./build.sh --config release

# List of available command-line options and their default values.
# Build for x64 architecture by default.
# Currently, only x64 architecture is supported.
architecture='x64'
# Build in Debug configuration by default.
configuration='Debug'
# Name of CMake executable (usable for script debugging).
cmake='cmake'

use_ninja=0
cxx_compiler=''
build_dir_suffix=''

# Iterate through the command-line arguments and extract parameters if any.
args=()
while (($#)); do
  case "$1" in
    --arch=* ) architecture="${1#*=}"; shift;;
    --arch   ) architecture="${2}"; shift 2;;

    --config=* ) configuration="${1#*=}"; shift;;
    --config   ) configuration="${2}"; shift 2;;

    --cmake=* ) cmake="${1#*=}"; shift;;
    --cmake   ) cmake="${2}"; shift 2;;

    --ninja     ) use_ninja=1; shift ;;

    --compiler=* ) cxx_compiler="${1#*=}"; shift;;
    --compiler   ) cxx_compiler="${2}"; shift 2;;

    --suffix=* ) build_dir_suffix="${1#*=}"; shift;;
    --suffix   ) build_dir_suffix="${2}"; shift 2;;
   * ) args+=("${1}"); shift;;
  esac
done

if [[ "$cxx_compiler" == "" ]]; then
  cxx_compiler="$CXX"
fi

# Set configuration-dependent parameters.
if [ "$(echo "${configuration}" | tr '[:upper:]' '[:lower:]')" = "debug" ]
then
  # Limit the number of parallel builds to 1 (no parallel builds for Debug configuration).
  parallel=1
else
  # Set the number of parallel builds to 4 for Release configuration.
  parallel=4
fi

if [ ${use_ninja} == 1 ]
then
  cmake_defines="${cmake_defines} -G Ninja"
fi

# Get script root folder.
script_dir="$(cd $(dirname $0); pwd)"
# Set needed folders.
root_dir="${script_dir}"
project_dir="${root_dir}"

# Set parameters-dependent path to `build` folder (resulted binary will be placed there).
cmake_output_dir=$(${root_dir}/support/build/getBuildDirName.sh "$architecture" "$cxx_compiler" "$configuration" "$build_dir_suffix")
if [[ $? -ne 0 ]]; then
    exit 1
fi


# Build SGParser project with CMake.
CXX=$cxx_compiler "${cmake}" -S "${project_dir}" -B "${cmake_output_dir}" -DCMAKE_BUILD_TYPE="${configuration}" ${cmake_defines}
"${cmake}" --build "${cmake_output_dir}" --config "${configuration}" --parallel "${parallel}"
