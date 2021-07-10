# top level project rules for the msm8916 project
#
LOCAL_DIR := $(GET_LOCAL_DIR)


include $(LOCAL_DIR)/msm8994.mk
include $(LOCAL_DIR)/lk2nd.mk

# Feel free to enable that and fix compile errors
# if your 8992/4 device has UFS for some reason
ENABLE_UFS_SUPPORT   := 0

DEFINES += ENABLE_FBCON_LOGGING=1
DEFINES += WITH_DEBUG_LOG_BUF=1

#Enable the feature of long press power on
DEFINES += LONG_PRESS_POWER_ON=1

# Use continuous splash from primary bootloader for display
DISPLAY_USE_CONTINUOUS_SPLASH := 1

APPSBOOTHEADER: $(OUTBOOTIMG) $(OUTBOOTIMGADTB)
ANDROID_BOOT_BASE := 0
ANDROID_BOOT_CMDLINE := lk2nd
