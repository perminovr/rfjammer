TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += network
LIBS +=

HEADERS += \
	qwframehandler.h


SOURCES += \
	qwframehandler.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/qwframehandler


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

