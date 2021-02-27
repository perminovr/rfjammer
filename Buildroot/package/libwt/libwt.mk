################################################################################
#
# LIBWT
#
################################################################################

LIBWT_VERSION = 4.4
LIBWT_SITE = https://github.com/emweb/wt/archive
LIBWT_SOURCE = $(LIBWT_VERSION)-release.zip
LIBWT_SOURCE_FOLDER = wt-$(LIBWT_VERSION)-release
LIBWT_INSTALL_STAGING = YES
LIBWT_LICENSE = GPLv2
LIBWT_LICENSE_FILES = COPYING

define LIBWT_EXTRACT_CMDS
	unzip -qq $(DL_DIR)/$(LIBWT_SOURCE) -d $(@D)
	mv $(@D)/$(LIBWT_SOURCE_FOLDER)/* $(@D)
	rm -rf $(@D)/$(LIBWT_SOURCE_FOLDER)
endef

ifeq ($(BR2_PACKAGE_BOOST),y)
LIBWT_CONF_OPTS += 
LIBWT_DEPENDENCIES += boost
endif

$(eval $(cmake-package))
