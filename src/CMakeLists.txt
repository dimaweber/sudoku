set (SOURCES main.cpp
	fieldgui.cpp)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(OpenGL REQUIRED)
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)

include_directories(../libsudoku)

qt_add_executable(sudoku ${SOURCES})

target_link_libraries(sudoku PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets solver -lgsl)

install(TARGETS sudoku RUNTIME)
