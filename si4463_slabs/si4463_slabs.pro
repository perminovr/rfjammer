TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += 
LIBS += 

HEADERS += \
	radio_config_Si4463.h \
	si446x_patch.h \
	si4463_slabs.h \

SOURCES += \
	si4463_slabs.cpp \


include(../root.pri)


TARGET = $${PRJ_LIB}/si4463_slabs


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
