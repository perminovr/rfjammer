TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT +=
LIBS +=

HEADERS += \
	ibus.h \
	spibus.h \
	iradiohandler.h \
	iradiochip.h \
	si4463.h \
	radiohandler.h \


SOURCES += \
	si4463.cpp \
	radiohandler.cpp \


include(../root.pri)
include(../linuxutils/linuxutils.pri)
include(../si4463_slabs/si4463_slabs.pri)


TARGET = $${PRJ_LIB}/radiodrv


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

