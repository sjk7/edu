TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++1z # latest and greatest c++ version.

CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += NDEBUG
} else {
    #This is a debug build
}


SOURCES += \
    src/db-test.cpp

HEADERS += \
    include/db.h \
    include/collection.h \
    include/runtime_error.h \
    include/strings.h \
    include/macros.h \
    include/dbtypes.h \
    include/dbcolumns.h \
    include/dbrows.h \
    include/dbheader.h \
    include/num.h \
    include/text_importer.h \
    include/utils.h
