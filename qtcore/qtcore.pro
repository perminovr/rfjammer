TEMPLATE = app

QT -= gui
QT += network core

CONFIG += console
CONFIG -= app_bundle


HEADERS += \
	prgdatabase.h \
	ovpnprovider.h \
	udhcpprovider.h \
	qtworker.h \
	rfjammersctl.h \
	ctlserver.h \
	slaveemulator.h \
    statusdrv.h \


SOURCES += \
	prgdatabase.cpp \
	ovpnprovider.cpp \
	udhcpprovider.cpp \
	qtworker.cpp \
	main.cpp \
	rfjammersctl.cpp \
	ctlserver.cpp \


include(../root.pri)
include(../framehandler/framehandler.pri)
include(../qwframehandler/qwframehandler.pri)
include(../idhcpclient/idhcpclient.pri)
include(../networking/networking.pri)
include(../linuxutils/linuxutils.pri)
include(../si4463_slabs/si4463_slabs.pri)
include(../radio/radio.pri)
include(../ivpnclient/ivpnclient.pri)


TARGET = $${PRJ_BIN}/qtcore


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
