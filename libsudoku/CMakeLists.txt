set (SOURCES        bilocationlink.cpp
                    cellcolor.cpp
                    cell.cpp
                    coord.cpp
                    field.cpp
                    house.cpp
                    resolver.cpp
                    technique.cpp
        )

set(CMAKE_AUTOMOC ON)

add_compile_definitions(SUDOKU_LIBRARY)
add_compile_definitions(_DELAY_SET_VALUE)
add_compile_definitions(_DELAY_TECHNIQUE_RUN)
add_compile_definitions(LOG_STREAM=std::clog)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)
qt_standard_project_setup()

qt_add_library(libsudoku ${SOURCES})

target_link_libraries(libsudoku PRIVATE Qt6::Core Qt6::Concurrent)

install(TARGETS libsudoku LIBRARY)
