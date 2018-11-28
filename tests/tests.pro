QT       += testlib sql
QT       -= gui

TARGET = unit_tests
CONFIG   += console c++14
CONFIG   -= app_bundle

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DESTDIR = ../bin

SOURCES += \
	tests.cpp \
        ../src/coord.cpp	

DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -lgsl -lgslcblas

INCLUDEPATH += ../src 

OBJECTS_DIR=.obj
MOC_DIR=.moc

DESTDIR=../bin
