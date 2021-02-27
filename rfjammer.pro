TEMPLATE = subdirs
CONFIG += c++11
CONFIG += ordered

CONFIG += create_pc
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

# lib
PRJLIBS = \
	common \
	linuxutils \
	ivpnclient \
	idhcpclient \
	networking \
	si4463_slabs \
	radio \
	framehandler \
	qwframehandler \

# exec
PRJEXEC = \
	dhcpnotifier \
	wtgui \
	qtcore \

SUBDIRS += $${PRJLIBS} $${PRJEXEC}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
