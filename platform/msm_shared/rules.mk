LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += \
			-I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/dev/panel/msm -I$(LK_TOP_DIR)/app/aboot

DEFINES += $(TARGET_XRES)
DEFINES += $(TARGET_YRES)

OBJS += \
	$(LOCAL_DIR)/debug.o \
	$(LOCAL_DIR)/smem.o \
	$(LOCAL_DIR)/smem_ptable.o \
	$(LOCAL_DIR)/jtag_hook.o \
	$(LOCAL_DIR)/jtag.o \
	$(LOCAL_DIR)/partition_parser.o \
	$(LOCAL_DIR)/hsusb.o \
	$(LOCAL_DIR)/boot_stats.o \
	$(LOCAL_DIR)/crc32.o

ifeq ($(ENABLE_WDOG_SUPPORT),1)
OBJS += \
	$(LOCAL_DIR)/wdog.o
endif

ifeq ($(ENABLE_SECAPP_LOADER), 1)
OBJS += $(LOCAL_DIR)/secapp_loader.o
endif

ifeq ($(ENABLE_QGIC3), 1)
OBJS += $(LOCAL_DIR)/qgic_v3.o
endif

ifeq ($(ENABLE_SMD_SUPPORT),1)
OBJS += \
	$(LOCAL_DIR)/rpm-smd.o \
	$(LOCAL_DIR)/smd.o
endif
ifeq ($(ENABLE_SDHCI_SUPPORT),1)
OBJS += \
	$(LOCAL_DIR)/sdhci.o \
	$(LOCAL_DIR)/sdhci_msm.o \
	$(LOCAL_DIR)/mmc_sdhci.o \
	$(LOCAL_DIR)/mmc_wrapper.o
else
OBJS += \
	$(LOCAL_DIR)/mmc.o
endif

ifeq ($(VERIFIED_BOOT),1)
OBJS += \
	$(LOCAL_DIR)/boot_verifier.o
endif

ifeq ($(ENABLE_FBCON_DISPLAY_MSG),1)
OBJS += \
	$(LOCAL_DIR)/menu_keys_detect.o \
	$(LOCAL_DIR)/display_menu.o
endif

ifeq ($(PLATFORM),msm8x60)
	OBJS += $(LOCAL_DIR)/mipi_dsi.o \
			$(LOCAL_DIR)/i2c_qup.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/crypto_eng.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/lcdc.o \
			$(LOCAL_DIR)/mddi.o \
			$(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/mdp4.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/hdmi.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/timer.o \
			$(LOCAL_DIR)/nand.o
endif

ifeq ($(PLATFORM),msm8960)
	OBJS += $(LOCAL_DIR)/hdmi.o \
			$(LOCAL_DIR)/mipi_dsi.o \
			$(LOCAL_DIR)/i2c_qup.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/mdp4.o \
			$(LOCAL_DIR)/crypto4_eng.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/clock-local.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/display.o \
			$(LOCAL_DIR)/lvds.o \
			$(LOCAL_DIR)/mipi_dsi_phy.o \
			$(LOCAL_DIR)/timer.o \
			$(LOCAL_DIR)/mdp_lcdc.o \
			$(LOCAL_DIR)/nand.o
endif

ifeq ($(PLATFORM),msm8974)
DEFINES += DISPLAY_TYPE_MDSS=1
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/mdp5.o \
			$(LOCAL_DIR)/display.o \
			$(LOCAL_DIR)/mipi_dsi.o \
			$(LOCAL_DIR)/mipi_dsi_phy.o \
			$(LOCAL_DIR)/mipi_dsi_autopll.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/crypto5_eng.o \
			$(LOCAL_DIR)/crypto5_wrapper.o \
			$(LOCAL_DIR)/i2c_qup.o \
			$(LOCAL_DIR)/gpio.o \
			$(LOCAL_DIR)/dload_util.o \
			$(LOCAL_DIR)/edp.o \
			$(LOCAL_DIR)/edp_util.o \
			$(LOCAL_DIR)/edp_aux.o \
			$(LOCAL_DIR)/edp_phy.o
endif

ifeq ($(PLATFORM),msm8226)
DEFINES += DISPLAY_TYPE_MDSS=1
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/mdp5.o \
			$(LOCAL_DIR)/display.o \
			$(LOCAL_DIR)/mipi_dsi.o \
			$(LOCAL_DIR)/mipi_dsi_phy.o \
			$(LOCAL_DIR)/mipi_dsi_autopll.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/qpic_nand.o \
            $(LOCAL_DIR)/certificate.o \
            $(LOCAL_DIR)/image_verify.o \
            $(LOCAL_DIR)/crypto_hash.o \
            $(LOCAL_DIR)/crypto5_eng.o \
            $(LOCAL_DIR)/crypto5_wrapper.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/gpio.o \
			$(LOCAL_DIR)/dload_util.o \
			$(LOCAL_DIR)/shutdown_detect.o
endif

ifeq ($(PLATFORM),msm8916)
DEFINES += DISPLAY_TYPE_MDSS=1
	OBJS += $(LOCAL_DIR)/qgic.o \
		$(LOCAL_DIR)/qtimer.o \
		$(LOCAL_DIR)/qtimer_mmap.o \
		$(LOCAL_DIR)/interrupts.o \
		$(LOCAL_DIR)/clock.o \
		$(LOCAL_DIR)/clock_pll.o \
		$(LOCAL_DIR)/clock_lib2.o \
		$(LOCAL_DIR)/uart_dm.o \
		$(LOCAL_DIR)/board.o \
		$(LOCAL_DIR)/spmi.o \
		$(LOCAL_DIR)/bam.o \
		$(LOCAL_DIR)/scm.o \
		$(LOCAL_DIR)/qpic_nand.o \
		$(LOCAL_DIR)/dload_util.o \
		$(LOCAL_DIR)/gpio.o \
		$(LOCAL_DIR)/dev_tree.o \
                $(LOCAL_DIR)/qseecom_lk.o \
		$(LOCAL_DIR)/mdp5.o \
		$(LOCAL_DIR)/display.o \
		$(LOCAL_DIR)/mipi_dsi.o \
		$(LOCAL_DIR)/mipi_dsi_phy.o \
		$(LOCAL_DIR)/mipi_dsi_autopll.o \
		$(LOCAL_DIR)/shutdown_detect.o \
		$(LOCAL_DIR)/certificate.o \
		$(LOCAL_DIR)/image_verify.o \
		$(LOCAL_DIR)/crypto_hash.o \
		$(LOCAL_DIR)/crypto5_eng.o \
		$(LOCAL_DIR)/crypto5_wrapper.o \
		$(LOCAL_DIR)/i2c_qup.o

endif


ifeq ($(PLATFORM),msm8610)
DEFINES += DISPLAY_TYPE_MDSS=1
    OBJS += $(LOCAL_DIR)/qgic.o \
            $(LOCAL_DIR)/qtimer.o \
            $(LOCAL_DIR)/qtimer_mmap.o \
            $(LOCAL_DIR)/interrupts.o \
            $(LOCAL_DIR)/clock.o \
            $(LOCAL_DIR)/clock_pll.o \
            $(LOCAL_DIR)/clock_lib2.o \
            $(LOCAL_DIR)/uart_dm.o \
            $(LOCAL_DIR)/board.o \
            $(LOCAL_DIR)/display.o \
            $(LOCAL_DIR)/mipi_dsi.o \
            $(LOCAL_DIR)/mipi_dsi_phy.o \
            $(LOCAL_DIR)/mdp3.o \
            $(LOCAL_DIR)/spmi.o \
            $(LOCAL_DIR)/bam.o \
            $(LOCAL_DIR)/qpic_nand.o \
            $(LOCAL_DIR)/dev_tree.o \
            $(LOCAL_DIR)/scm.o \
            $(LOCAL_DIR)/gpio.o \
            $(LOCAL_DIR)/certificate.o \
            $(LOCAL_DIR)/image_verify.o \
            $(LOCAL_DIR)/crypto_hash.o \
            $(LOCAL_DIR)/crypto5_eng.o \
            $(LOCAL_DIR)/crypto5_wrapper.o \
            $(LOCAL_DIR)/dload_util.o \
            $(LOCAL_DIR)/shutdown_detect.o
endif

ifeq ($(PLATFORM),apq8084)
DEFINES += DISPLAY_TYPE_MDSS=1
    OBJS += $(LOCAL_DIR)/qgic.o \
            $(LOCAL_DIR)/qtimer.o \
            $(LOCAL_DIR)/qtimer_mmap.o \
            $(LOCAL_DIR)/interrupts.o \
            $(LOCAL_DIR)/clock.o \
            $(LOCAL_DIR)/clock_pll.o \
            $(LOCAL_DIR)/clock_lib2.o \
            $(LOCAL_DIR)/uart_dm.o \
            $(LOCAL_DIR)/board.o \
            $(LOCAL_DIR)/mdp5.o \
            $(LOCAL_DIR)/display.o \
            $(LOCAL_DIR)/mipi_dsi.o \
            $(LOCAL_DIR)/mipi_dsi_phy.o \
            $(LOCAL_DIR)/mipi_dsi_autopll.o \
            $(LOCAL_DIR)/mdss_hdmi.o \
            $(LOCAL_DIR)/hdmi_pll_28nm.o \
            $(LOCAL_DIR)/spmi.o \
            $(LOCAL_DIR)/bam.o \
            $(LOCAL_DIR)/qpic_nand.o \
            $(LOCAL_DIR)/dev_tree.o \
            $(LOCAL_DIR)/gpio.o \
            $(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/ufs.o \
			$(LOCAL_DIR)/utp.o \
			$(LOCAL_DIR)/uic.o \
			$(LOCAL_DIR)/ucs.o \
			$(LOCAL_DIR)/ufs_hci.o \
			$(LOCAL_DIR)/dme.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/crypto5_eng.o \
			$(LOCAL_DIR)/crypto5_wrapper.o \
			$(LOCAL_DIR)/edp.o \
			$(LOCAL_DIR)/edp_util.o \
			$(LOCAL_DIR)/edp_aux.o \
			$(LOCAL_DIR)/edp_phy.o

endif

ifeq ($(PLATFORM),msm7x27a)
	OBJS += $(LOCAL_DIR)/uart.o \
			$(LOCAL_DIR)/nand.o \
			$(LOCAL_DIR)/proc_comm.o \
			$(LOCAL_DIR)/mdp3.o \
			$(LOCAL_DIR)/mipi_dsi.o \
			$(LOCAL_DIR)/crypto_eng.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/timer.o \
			$(LOCAL_DIR)/display.o \
			$(LOCAL_DIR)/mipi_dsi_phy.o \
			$(LOCAL_DIR)/mdp_lcdc.o \
			$(LOCAL_DIR)/spi.o
endif

ifeq ($(PLATFORM),msm7k)
	OBJS += $(LOCAL_DIR)/uart.o \
			$(LOCAL_DIR)/nand.o \
			$(LOCAL_DIR)/proc_comm.o \
			$(LOCAL_DIR)/lcdc.o \
			$(LOCAL_DIR)/mddi.o \
			$(LOCAL_DIR)/timer.o
endif

ifeq ($(PLATFORM),msm7x30)
	OBJS += $(LOCAL_DIR)/crypto_eng.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/uart.o \
			$(LOCAL_DIR)/nand.o \
			$(LOCAL_DIR)/proc_comm.o \
			$(LOCAL_DIR)/lcdc.o \
			$(LOCAL_DIR)/mddi.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/timer.o
endif

ifeq ($(PLATFORM),mdm9x15)
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/nand.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/timer.o
endif

ifeq ($(PLATFORM),mdm9x25)
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o
endif

ifeq ($(PLATFORM),mdm9x35)
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/qmp_usb30_phy.o
endif

ifeq ($(PLATFORM),msmzirc)
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/gpio.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/qmp_usb30_phy.o \
			$(LOCAL_DIR)/qusb2_phy.o
endif

ifeq ($(PLATFORM),fsm9900)
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/crypto5_eng.o \
			$(LOCAL_DIR)/crypto5_wrapper.o \
			$(LOCAL_DIR)/i2c_qup.o \
			$(LOCAL_DIR)/gpio.o \
			$(LOCAL_DIR)/dload_util.o
endif

ifeq ($(PLATFORM),fsm9010)
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/crypto5_eng.o \
			$(LOCAL_DIR)/crypto5_wrapper.o \
			$(LOCAL_DIR)/i2c_qup.o \
			$(LOCAL_DIR)/gpio.o \
			$(LOCAL_DIR)/dload_util.o
endif

ifeq ($(PLATFORM),msm8994)
DEFINES += DISPLAY_TYPE_MDSS=1
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/gpio.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/ufs.o \
			$(LOCAL_DIR)/utp.o \
			$(LOCAL_DIR)/uic.o \
			$(LOCAL_DIR)/ucs.o \
			$(LOCAL_DIR)/ufs_hci.o \
			$(LOCAL_DIR)/dme.o \
			$(LOCAL_DIR)/qmp_usb30_phy.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/crypto5_eng.o \
			$(LOCAL_DIR)/crypto5_wrapper.o \
			$(LOCAL_DIR)/qusb2_phy.o \
			$(LOCAL_DIR)/mdp5.o \
			$(LOCAL_DIR)/display.o \
			$(LOCAL_DIR)/mipi_dsi.o \
			$(LOCAL_DIR)/mipi_dsi_phy.o \
			$(LOCAL_DIR)/mipi_dsi_autopll.o \
			$(LOCAL_DIR)/mipi_dsi_autopll_20nm.o \
			$(LOCAL_DIR)/i2c_qup.o \
			$(LOCAL_DIR)/shutdown_detect.o \
			$(LOCAL_DIR)/dload_util.o
endif

ifeq ($(PLATFORM),msm8909)
DEFINES += DISPLAY_TYPE_MDSS=1
	OBJS += $(LOCAL_DIR)/qgic.o \
			$(LOCAL_DIR)/qtimer.o \
			$(LOCAL_DIR)/qtimer_mmap.o \
			$(LOCAL_DIR)/interrupts.o \
			$(LOCAL_DIR)/clock.o \
			$(LOCAL_DIR)/clock_pll.o \
			$(LOCAL_DIR)/clock_lib2.o \
			$(LOCAL_DIR)/uart_dm.o \
			$(LOCAL_DIR)/board.o \
			$(LOCAL_DIR)/spmi.o \
			$(LOCAL_DIR)/bam.o \
			$(LOCAL_DIR)/qpic_nand.o \
			$(LOCAL_DIR)/flash-ubi.o \
			$(LOCAL_DIR)/scm.o \
			$(LOCAL_DIR)/dev_tree.o \
			$(LOCAL_DIR)/gpio.o \
			$(LOCAL_DIR)/crypto_hash.o \
			$(LOCAL_DIR)/crypto5_eng.o \
			$(LOCAL_DIR)/crypto5_wrapper.o \
			$(LOCAL_DIR)/dload_util.o \
			$(LOCAL_DIR)/shutdown_detect.o \
			$(LOCAL_DIR)/certificate.o \
			$(LOCAL_DIR)/image_verify.o \
			$(LOCAL_DIR)/i2c_qup.o \
                        $(LOCAL_DIR)/qseecom_lk.o \
			$(LOCAL_DIR)/mdp3.o \
			$(LOCAL_DIR)/display.o \
			$(LOCAL_DIR)/mipi_dsi.o \
			$(LOCAL_DIR)/mipi_dsi_phy.o \
			$(LOCAL_DIR)/mipi_dsi_autopll.o
endif


ifeq ($(ENABLE_UFS_SUPPORT), 1)
	OBJS += \
			$(LOCAL_DIR)/ufs.o \
			$(LOCAL_DIR)/utp.o \
			$(LOCAL_DIR)/uic.o \
			$(LOCAL_DIR)/ucs.o \
			$(LOCAL_DIR)/ufs_hci.o \
			$(LOCAL_DIR)/dme.o
endif

ifeq ($(ENABLE_BOOT_CONFIG_SUPPORT), 1)
	OBJS += \
		$(LOCAL_DIR)/boot_device.o
endif

ifeq ($(ENABLE_USB30_SUPPORT),1)
	OBJS += \
		$(LOCAL_DIR)/usb30_dwc.o \
		$(LOCAL_DIR)/usb30_dwc_hw.o \
		$(LOCAL_DIR)/usb30_udc.o \
		$(LOCAL_DIR)/usb30_wrapper.o
endif

ifeq ($(ENABLE_REBOOT_MODULE), 1)
	OBJS += $(LOCAL_DIR)/reboot.o
endif

ifeq ($(ENABLE_RPMB_SUPPORT), 1)
include platform/msm_shared/rpmb/rules.mk
ifeq ($(ENABLE_UFS_SUPPORT), 1)
        OBJS += $(LOCAL_DIR)/rpmb_ufs.o
endif
endif

ifneq ($(VERIFIED_BOOT),1)
# Why do we need stupid keystore stuff with verified boot disabled?
DEFINES := $(filter-out SSD_ENABLE TZ_SAVE_KERNEL_HASH, $(DEFINES))

# Meh. Who needs crypto? I have nothing to hide!!!!
unneeded_objs := \
	$(LOCAL_DIR)/certificate.o \
	$(LOCAL_DIR)/image_verify.o \
	$(LOCAL_DIR)/crypto_hash.o \
	$(LOCAL_DIR)/crypto5_eng.o \
	$(LOCAL_DIR)/crypto5_wrapper.o
OBJS := $(filter-out $(unneeded_objs), $(OBJS))
endif
