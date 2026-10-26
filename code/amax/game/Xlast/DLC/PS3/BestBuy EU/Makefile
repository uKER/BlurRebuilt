#   SCE CONFIDENTIAL
#   PLAYSTATION(R)3 Programmer Tool Runtime Library 270.001
#   Copyright (C) 2006 Sony Computer Entertainment Inc.
#   All Rights Reserved.

CELL_SDK ?= /usr/local/cell
CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

OBJS_DIR = USRDIR

PACKAGER = $(CELL_HOST_PATH)/bin/make_package_npdrm
MAKEEDAT = $(CELL_HOST_PATH)/bin/make_edata_npdrm

PKG_CONFIG = package.conf
PKG_TARGET = $(shell $(PACKAGER) --print-content-id $(PKG_CONFIG)).pkg
SAMPLE_PNG = sample.png

EXTRA_TARGET = $(PKG_TARGET)
CLEANFILES = $(PKG_TARGET) $(OBJS_DIR)/*

$(PKG_TARGET): $(PKG_CONFIG) $(OBJS_DIR)/sample.edat $(OBJS_DIR)/sample.png
	@echo \# generate sample npdrm package in here.
	$(PACKAGER) $< .

$(OBJS_DIR)/sample.edat: $(SAMPLE_PNG)
	@echo \# make npdrm edata in $(OBJS_DIR)/.
	@mkdir -p $(@D)
	$(MAKEEDAT) $< $@

$(OBJS_DIR)/sample.png: $(SAMPLE_PNG)
	@echo \# copy $< to $(OBJS_DIR)/.
	@mkdir -p $(@D)
	$(CP) $< $@

include $(CELL_MK_DIR)/sdk.target.mk