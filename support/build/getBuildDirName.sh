#!/usr/bin/env bash

# Filename:  getBuildDirName.sh
# Content:   Bash script for getting build folder name
# Provided AS IS under MIT License; see LICENSE file in root folder.
#
# This is helper script, it's not supposed to be directly used,
# The script is not doing its best to parse parameters really safely.
# The script relies hard on order of parameters and soes minimum efforts to validate them.
#
# Usage: getBuildDirName.sh "[x86|x64]" "<COMPILER>" "[debug|release]" "(optional)<SUFFIX>"
#
# Returns: a full path to build folder for given set of parameters.
# Folder name is defined as: <ARCH>-<TOOLKIT>-<CONFIG>[-<SUFFIX>]
# Where <TOOLKIT> is a result of execution of getToolkitId.sh "<COMPILER>"
#
# Examples: assuming current directory is /a/b/c/XLang, getBuildDirName.sh with the following
# parameters gives result:
#  getBuildDirName.sh "x64" "g++" "release" -> /a/b/c/XLang/build/x64-gcc-release
#  getBuildDirName.sh "x64" "/usr/local/bin/clang++" "debug" -> /a/b/c/XLang/build/x64-clang-debug
#  getBuildDirName.sh "x86" "msvc" "debug" "llvm"-> /a/b/c/XLang/build/x86-msvc-debug-llvm

PLATFORM=$1
COMPILER=$2
CONFIG=$3
SUFFIX=$4

MY_DIR=$(cd $(dirname $0); pwd)
WORKSPACE_DIR=$(cd $MY_DIR/../..; pwd)

#`die` prints an error-msg and exits (exit-code 1).
die() {
    #join "$*" with spaces
    IFS=' '
    printf '%s\n' "$*" 1>&2
    exit 1
}

toolkit_id=$( ${MY_DIR}/getToolkitId.sh $COMPILER )
[[ $? -eq 0 ]] || die "Can't get toolkit id for CXX=\"${COMPILER}\""

dirname_base=$( echo "$PLATFORM-$toolkit_id-$CONFIG"| tr '[:upper:]' '[:lower:]' )
dir_path="$WORKSPACE_DIR/build/$dirname_base"

if [[ "$SUFFIX" != "" ]]; then
    dir_path="$dir_path-$SUFFIX"
fi

echo $dir_path
