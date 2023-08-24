set (SOURCES tests.cpp
    )

find_package(Qt6 REQUIRED COMPONENTS Core Test )
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include_directories(../libsudoku)

qt_add_executable(unit_tests ${SOURCES} resource.qrc)

target_link_libraries(unit_tests PRIVATE Qt6::Core Qt6::Test solver -lgsl)

install(TARGETS unit_tests RUNTIME)
