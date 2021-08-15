include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=openwrt_makefile
PKG_VERSION:=1.0
PKG_RELEASE:=1

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=.

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -I./include
TARGET_CFLAGS += -fPIC
TARGET_LDFLAGS += -shared

define Package/openwrt_makefile
  SECTION:=wucg
  CATEGORY:=WUCG
  TITLE:=TEST library.
endef

define Package/openwrt_makefile/description
	WiFi and LTE network management library.
endef

define Build/Prepare
	echo "Compile libwal start."
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
# define Build/Compile
# 	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/test_interface.o -c $(PKG_BUILD_DIR)/src/test_interface.c
# 	$(TARGET_CC) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/libfun_so.so $(PKG_BUILD_DIR)/test_interface.o 
# endef

#make的真正执行内容，这里执行子目录
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include"
endef

#ipk安装包中的东东，执行ipk安装时会安装再固件环境中
define Package/openwrt_makefile/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/
endef

#这里应该是包相关的头和so安装到我们的交叉编译环境中
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/usr/lib
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,openwrt_makefile))

#  make -j V=s package/openwrt_makefile/compile