#!/usr/bin/env bash

# Filename:  generate_rus.sh
# Content:   Script that generates calculator parser files for grammar uses Russian language
# Provided AS IS under MIT License; see LICENSE file in root folder.

script_dir="$(cd $(dirname $0); pwd)"

"${script_dir}/generate.sh" 'src/Calc_rus.gr' ${@}
