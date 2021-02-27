TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT +=
LIBS +=

HEADERS += \
	devstructs.h \
	servstructs.h \
	config.h \
	staticmembers.h \
	common.h


SOURCES += \


include(../root.pri)


TARGET = $${PRJ_LIB}/common


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

