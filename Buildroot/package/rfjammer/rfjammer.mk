################################################################################
#
# rfjammer
#
################################################################################

RFJAMMER_VERSION = 0.1
RFJAMMER_SOURCE = rfjammer-$(RFJAMMER_VERSION).tar.xz
RFJAMMER_SITE = $(TOPDIR)/RFJammer
RFJAMMER_SITE_METHOD = file
RFJAMMER_DEPENDENCIES = qt5base libwt libnl
RFJAMMER_LICENSE = DWTFYWWI
RFJAMMER_LICENSE_FILES = LICENSE

define RFJAMMER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 755 -D -d $(TARGET_DIR)/home/root/Lib
	$(INSTALL) -m 755 -D -d $(TARGET_DIR)/home/root/Bin
	$(INSTALL) -m 755 -D -d $(TARGET_DIR)/home/root/Scripts
	$(INSTALL) -m 755 -D -d $(TARGET_DIR)/home/root/Wtgui/approot
	$(INSTALL) -m 755 -D -d $(TARGET_DIR)/home/root/Wtgui/docroot/pics
	$(INSTALL) -m 755 -D -d $(TARGET_DIR)/home/root/Wtgui/docroot/style

	$(INSTALL) -m 755 -D $(@D)/Files/example.ovpn		$(TARGET_DIR)/home/root/Conf/example.ovpn
	$(INSTALL) -m 755 -D $(@D)/Files/static-net.cfg		$(TARGET_DIR)/home/root/Conf/static-net.cfg
	$(INSTALL) -m 755 -D $(@D)/Files/RunTimeScripts/*	$(TARGET_DIR)/home/root/Scripts/
	$(INSTALL) -m 755 -D $(@D)/build/usr/lib/* 			$(TARGET_DIR)/home/root/Lib/
	$(INSTALL) -m 755 -D $(@D)/build/usr/bin/*			$(TARGET_DIR)/home/root/Bin/
	$(INSTALL) -m 755 -D $(@D)/wtgui/src/wrc/approot/*			$(TARGET_DIR)/home/root/Wtgui/approot/
	$(INSTALL) -m 755 -D $(@D)/wtgui/src/wrc/docroot/pics/*		$(TARGET_DIR)/home/root/Wtgui/docroot/pics/
	$(INSTALL) -m 755 -D $(@D)/wtgui/src/wrc/docroot/style/*	$(TARGET_DIR)/home/root/Wtgui/docroot/style/
endef

define RFJAMMER_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D $(@D)/Files/rfjammer-startup.sh		$(TARGET_DIR)/etc/init.d/S99rfjammer-startup
	$(INSTALL) -m 755 -D $(@D)/Files/rfjammer-interfaces.sh		$(TARGET_DIR)/etc/init.d/S02rfjammer-startup
endef

$(eval $(qmake-package))
