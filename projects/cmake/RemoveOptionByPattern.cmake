# Filename:  RemoveOptionByPattern.cmake
# Content:   Helper function to remove options from strings.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Example of usage:
# set(var1 "/123 /xxx")
# set(var2 "-yyy -789")
# RemoveOptionByPattern("[0-9]+" var1, var2)
# leads to var1="/xxx", var2="-yyy"
function(RemoveOptionByPattern PATTERN)
    foreach(v ${ARGN})
        set(FULL_PATTERN "[-/]${PATTERN}[ \t]?")
        string(REGEX REPLACE ${FULL_PATTERN} "" TEMP "${${v}}")
        set("${v}" ${TEMP} PARENT_SCOPE)
    endforeach()
endfunction()
