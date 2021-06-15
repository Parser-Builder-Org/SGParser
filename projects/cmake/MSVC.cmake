# Filename:  MSVC.cmake
# Content:   CMake options for MSVC compiler.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set Windows 10 as a target platform version.
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

include("${CMAKE_CURRENT_LIST_DIR}/RemoveOptionByPattern.cmake")

####################################################################################################
# Set CONFORMANCE options for all configurations:
#
# /permissive-          - (also sets /Zc:referenceBinding, /Zc:strictStrings, and /Zc:rvalueCast).
# /Zc:externConstexpr   - enables external linkage for constexpr variables.
# /Zc:inline            - enforces the C++11 requirement that all functions declared inline
#                         must have a definition available in the same translation unit if they are
#                         used.
# /Zc:referenceBinding  - (implied by /permissive-) disallows binding non-const reference to
#                         temporary.
# /Zc:strictStrings     - (implied by /permissive-) requires strict const-qualification conformance
#                         for pointers initialized by using string literals.
# /Zc:rvalueCast        - (implied by /permissive-)  the compiler correctly identifies an rvalue
#                         reference type as the result of a cast operation.
# /Zc:throwingNew       - assumes operator new throws on failure (otherwise compiler inserts checks)
# /Zc:preprocessor      - (DISABLED FOR NOW) uses C11/17 conformant preprocessor.
set(XC_MSVC_CONFORMANCE_OPTIONS /permissive- /Zc:throwingNew /Zc:externConstexpr /Zc:inline)

####################################################################################################
# Set GENERIC options for all configurations:
# /W4   - enables warning level 4.
# /WX   - treats all compiler warnings as errors.
# /Za   - disables language extensions.
# /sdl- - disables Security Development Lifecycle (SDL) checks.
# /GR   - enables RTTI.
# /Gy   - enables function level linking.
set(XC_MSVC_GENERIC_OPTIONS /W4 /WX /Za /sdl- /GR /Gy)

####################################################################################################
# Set OPTIMIZED OPTIONS for all kinds of release configurations:
# /Ox  - enables the /O compiler options that favor speed. Implies /Ob /Oi /Ot /Oy.
# /Ob2 - (implied by Ox) allows the compiler to expand any function not explicitly marked for no
#        inlining.
# /Oi  - (implied by Ox) generates intrinsic functions.
# /Ot  - (implied by Ox) favors fast code.
# /Oy  - (implied by Ox) enables FramePointerOmission.
set(XC_MSVC_RELEASE_OPTIONS /Ox)

if(${CMAKE_BUILD_TYPE_LOWER} MATCHES "minsizerel")
    # Favors smaller code - add /Os after /Ox
    set(XC_MSVC_RELEASE_OPTIONS ${XC_MSVC_RELEASE_OPTIONS} /Os)
endif()

####################################################################################################
# Set DEBUG OPTIONS (Debug configuration only):
# /Od     - disables optimizations.
# /bigobj - increases the number of sections that an object file can contain.
set(XC_MSVC_DEBUG_OPTIONS /Od /bigobj)

####################################################################################################
# Apply some workarounds for CMake behavior.
####################################################################################################

# CMake inserts /W3 flag for MSVC toolkit. Let's remove it (we specify /W4 explicitly).
RemoveOptionByPattern("W3" CMAKE_CXX_FLAGS)

# CMake inserts optimization flags depending on configuration, we use our own, so clean /O? setting.
RemoveOptionByPattern("O[d12]" CMAKE_CXX_FLAGS_RELEASE
                               CMAKE_CXX_FLAGS_RELWITHDEBINFO
                               CMAKE_CXX_FLAGS_MINSIZEREL
                               CMAKE_CXX_FLAGS_DEBUG)

# CMake inserts /INCREMENTAL flag for MSVC toolkit. Let's remove it because we use link-time
# codegeneration which is incompatible with incremental.
RemoveOptionByPattern("INCREMENTAL[^ \t]*" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO)

####################################################################################################
# Finally set relevant options.
####################################################################################################

# Set common options for all configurations.
add_compile_options(${XC_MSVC_GENERIC_OPTIONS} ${XC_MSVC_CONFORMANCE_OPTIONS})

# Set configuration-specific options.
if(SG_RELEASE_BUILD)
    # Sets correct LTO settings specific to compilers.
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    add_compile_options(${XC_MSVC_RELEASE_OPTIONS})
else()
    add_compile_options(${XC_MSVC_DEBUG_OPTIONS})
endif()
