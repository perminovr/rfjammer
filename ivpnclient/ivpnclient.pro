TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT +=
LIBS +=

HEADERS += \
	ivpnclient.h


SOURCES += \
	ivpnclient.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/ivpnclient


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

