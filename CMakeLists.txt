cmake_minimum_required(VERSION 3.16)
project(sudoku VERSION 1.2 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_EXTENSIONS off)
set (CMAKE_CXX_STANDARD_REQUIRED on)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif (CCACHE_FOUND)

add_compile_options(
        -Werror
#        -Wall
#        -Wextra
)

option(TIDY_CHECKS "Run clang-tidy for statis analyze" OFF)
if (TIDY_CHECKS)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -config-file=${CMAKE_CURRENT_LIST_DIR}/.clang-tidy;
            --enable-check-profile;
    )
endif ()

include(CPack)
include(GNUInstallDirs)

set(CMAKE_INSTALL_BINDIR bin)
set(CMAKE_INSTALL_LIBDIR lib)

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(src)
add_subdirectory(libsudoku)
add_subdirectory(tests)

file(GLOB_RECURSE sdm_files "puzzle/*.sdm")
install(FILES ${sdm_files} DESTINATION puzzle/)
