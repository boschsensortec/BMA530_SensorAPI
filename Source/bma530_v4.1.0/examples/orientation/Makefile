COINES_INSTALL_PATH ?= ../../../..

EXAMPLE_FILE ?= orientation.c

API_LOCATION ?= ../..
COMMON_LOCATION ?= ..

C_SRCS += \
$(API_LOCATION)/bma5.c \
$(API_LOCATION)/bma530.c \
$(API_LOCATION)/bma530_features.c \
$(COMMON_LOCATION)/common/common.c

INCLUDEPATHS += \
$(API_LOCATION) \
$(COMMON_LOCATION)/common

include $(COINES_INSTALL_PATH)/coines.mk
