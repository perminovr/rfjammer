TEMPLATE = app

QT -= gui
QT += core network

LIBS += -lwt -lwthttp

CONFIG += console
CONFIG -= app_bundle


HEADERS += \
	wtwithqt/DispatchThread.h \
	wtwithqt/WQApplication.h \
	ctlclient.h \
	mapplication.h

SOURCES += \
	wtwithqt/DispatchThread.cpp \
	wtwithqt/WQApplication.cpp \
	ctlclient.cpp \
	mapplication.cpp \
	main.cpp


include(../root.pri)
include(../framehandler/framehandler.pri)
include(../qwframehandler/qwframehandler.pri)


TARGET = $${PRJ_BIN}/wtgui


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
