COINES_INSTALL_PATH ?= ../../../..

EXAMPLE_FILE ?= fifo_wm_8_bit_dedicated_frame_comp_en.c

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
COINES_BACKEND ?= COINES_BRIDGE
include $(COINES_INSTALL_PATH)/coines.mk