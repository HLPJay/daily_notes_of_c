include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt_main_makefile
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/openwrt_main_makefile
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=main_openwrr -- prints a snarky message
endef

define Package/openwrt_main_makefile/description
	If you can't figure out what this program does, you're probably
	brain-dead and need immediate medical attention.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r ./* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

#执行make编译动作，这里编译子目录
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include"
endef

#创建目录   并且拷贝相关资源到目录中  ipk打包文件中，会安装到固件环境对应目录
#注意 这里的可执行文件
define Package/openwrt_main_makefile/install
	echo "Here is Package/install"	
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/main_test $(1)/bin/
	$(INSTALL_DIR) $(1)/etc/wuug/conf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/conf/* $(1)/etc/wuug/conf
endef

$(eval $(call BuildPackage,openwrt_main_makefile))
	
# make -j V=s package/openwrt_main_makefile/compile