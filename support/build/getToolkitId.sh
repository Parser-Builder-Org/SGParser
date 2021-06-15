#!/usr/bin/env bash

# Filename:  getToolkitId.sh
# Content:   Bash script for getting a toolkit name by compiler name/path
# Provided AS IS under MIT License; see LICENSE file in root folder.
#
# Usage: getToolkitId.sh "<COMPILER>"
# The result (echoed) is uniform name of "toolkit" determined from <COMPILER> e.g.:
#    <COMPILER> -> <TOOLKIT>
#    "g++" -> "gcc"
#    "g++-11" -> "gcc"
#    "/usr/local/bin/g++" -> "gcc"  - this one in particular is useful for MacOS
#    "g++" -> "gcc"
#    similarly for clang:
#       "clang++|/usr/bin/clang++|..."  -> "clang"
#
#    "c++" -> "gcc" or "clang" depending to what `which c++` points to
#
#    "msvc" -> msvc
#    "" -> the sript will get value of $CXX. If it's empty, default system compiler 
#          will be detected by cmake (default system compiler)

#`die` prints an error-msg and exits (exit-code 1).
die() {
    #join "$*" with spaces
    IFS=' '
    printf '%s\n' "$*" 1>&2
    exit 1
}

COMPILER=$1
if [[ "$COMPILER" == "" ]]; then
    COMPILER="$CXX"
fi

if [[ "$COMPILER" == "msvc" ]]; then
    #skip detection
    echo 'msvc'
    exit 0
else
    compiler_id=$( (cd ; CXX=$COMPILER cmake --system-information 2>/dev/null | grep -E "\bCMAKE_CXX_COMPILER_ID\b" ) | sed -E "s/.*\"(.*)\".*/\1/"; exit ${PIPESTATUS[0]})
fi

if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    die 'Failed to get toolkit'
fi

case ${compiler_id} in

    AppleClang)
        echo 'appleclang'
        ;;

    Clang)
        echo "clang"
        ;;

    GNU)
        echo "gcc"
        ;;

    *)
        die 'Unknown compiler ID ${compiler_id}'
        ;;
esac

