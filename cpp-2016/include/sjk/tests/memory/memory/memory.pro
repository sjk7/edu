TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../test-memory.cpp


#QMAKE_CC = gcc-5
#QMAKE_CXX = g++-5
QMAKE_CXXFLAGS += -std=c++1y -std=c++1z -std=c++14

QMAKE_CC = clang-3.8
QMAKE_CXX = clang-3.8
