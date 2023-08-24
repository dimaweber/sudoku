set (SOURCES tests.cpp
    )

find_package(Qt6 REQUIRED COMPONENTS Core Test )
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include_directories(../libsudoku)

qt_add_executable(tests ${SOURCES} resource.qrc)

target_link_libraries(tests PRIVATE Qt6::Core Qt6::Test libsudoku -lgsl)

install(TARGETS tests RUNTIME)
