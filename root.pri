
PRJ_LIB = ../usr/lib
PRJ_BIN = ../usr/bin

QT -= gui

LIBS += \
	-L$${PRJ_LIB}

include(common/common.pri)


QMAKE_CFLAGS += -O0
QMAKE_CXXFLAGS += -O0


target.path = /run/prj
INSTALLS += target

QMAKE_CXXFLAGS += -Wno-implicit-fallthrough

QMAKE_CXXFLAGS += --coverage -fprofile-dir="/tmp/coverage"
QMAKE_LFLAGS += --coverage -fprofile-dir="/tmp/coverage"
