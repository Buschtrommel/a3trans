QT += core xml
QT -= gui

CONFIG += c++11

TARGET = a3trans
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += src/main.cpp \
    src/scriptparser.cpp \
    src/project.cpp \
    src/package.cpp \
    src/container.cpp \
    src/key.cpp \
    src/translation.cpp \
    src/stringtableparser.cpp \
    src/filewriter.cpp \
    src/xliffparser.cpp

HEADERS += \
    src/scriptparser.h \
    src/project.h \
    src/package.h \
    src/container.h \
    src/key.h \
    src/translation.h \
    src/stringtableparser.h \
    src/filewriter.h \
    src/xliffparser.h
