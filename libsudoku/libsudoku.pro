#-------------------------------------------------
#
# Project created by QtCreator 2018-12-07T16:05:40
#
#-------------------------------------------------

QT       -= gui

TARGET = sudoku
TEMPLATE = lib

CONFIG += c++14

win {
CONFIG += staticlib
}

DEFINES += SUDOKU_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
		coord.cpp \
		cell.cpp \
		house.cpp \
		bilocationlink.cpp \
		cellcolor.cpp \
		field.cpp \
		resolver.cpp \
		technique.cpp

HEADERS += \
		coord.h \
		cell.h \
		cellcolor.h \
		house.h \
		bilocationlink.h \
		field.h \
		libsudoku_global.h \
		resolver.h \
		technique.h

unix {
	target.path = /usr/lib
	INSTALLS += target

	LIBS += -lgsl -lgslcblas
	QMAKE_CXXFLAGS += -Wall -Wpedantic
}




DESTDIR=../bin

OBJECTS_DIR = .obj
UI_DIR = .ui
MOC_DIR = .moc
