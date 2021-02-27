TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT +=
LIBS +=

HEADERS += \
	qepoll.h \
	gpiodrv.h \


SOURCES += \


include(../root.pri)


TARGET = $${PRJ_LIB}/linuxutils


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

