TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT +=
LIBS +=

HEADERS += \
	framehandler.h


SOURCES += \
	framehandler.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/framehandler


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

