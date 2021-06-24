/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <debug.h>
#include <platform/iomap.h>
#include <platform/irqs.h>
#include <platform/gpio.h>
#include <reg.h>
#include <target.h>
#include <platform.h>
#include <dload_util.h>
#include <uart_dm.h>
#include <mmc.h>
#include <spmi.h>
#include <board.h>
#include <smem.h>
#include <baseband.h>
#include <dev/keys.h>
#include <pm8x41.h>
#include <pm8x41_regulator.h>
#include <crypto5_wrapper.h>
#include <hsusb.h>
#include <clock.h>
#include <partition_parser.h>
#include <scm.h>
#include <platform/clock.h>
#include <platform/gpio.h>
#include <stdlib.h>

#define HW_PLATFORM_8994_INTERPOSER    0x3

extern  bool target_use_signed_kernel(void);
static void set_sdc_power_ctrl();

static unsigned int target_id;
static uint32_t pmic_ver;

#if MMC_SDHCI_SUPPORT
struct mmc_device *dev;
#endif

#define PMIC_ARB_CHANNEL_NUM    0
#define PMIC_ARB_OWNER_ID       0

#define WDOG_DEBUG_DISABLE_BIT  17

#define CE_INSTANCE             2
#define CE_EE                   1
#define CE_FIFO_SIZE            64
#define CE_READ_PIPE            3
#define CE_WRITE_PIPE           2
#define CE_READ_PIPE_LOCK_GRP   0
#define CE_WRITE_PIPE_LOCK_GRP  0
#define CE_ARRAY_SIZE           20

#ifdef SSD_ENABLE
#define SSD_CE_INSTANCE_1       1
#define SSD_PARTITION_SIZE      8192
#endif

#define FASTBOOT_MODE           0x77665500

#define BOARD_SOC_VERSION1(soc_rev) (soc_rev >= 0x10000 && soc_rev < 0x20000)

#if MMC_SDHCI_SUPPORT
static uint32_t mmc_sdhci_base[] =
	{ MSM_SDC1_SDHCI_BASE, MSM_SDC2_SDHCI_BASE, MSM_SDC3_SDHCI_BASE, MSM_SDC4_SDHCI_BASE };
#endif

static uint32_t mmc_sdc_base[] =
	{ MSM_SDC1_BASE, MSM_SDC2_BASE, MSM_SDC3_BASE, MSM_SDC4_BASE };

static uint32_t mmc_sdc_pwrctl_irq[] =
	{ SDCC1_PWRCTL_IRQ, SDCC2_PWRCTL_IRQ, SDCC3_PWRCTL_IRQ, SDCC4_PWRCTL_IRQ };

void target_early_init(void)
{
#if WITH_DEBUG_UART
	uart_dm_init(1, 0, BLSP1_UART1_BASE);
#endif
}

uint32_t target_hw_interposer()
{
	return board_hardware_subtype() == HW_PLATFORM_8994_INTERPOSER ? 1 : 0;
}

#if WITH_LK2ND
int target_volume_up_old()
#else
/* Return 1 if vol_up pressed */
int target_volume_up()
#endif
{
	uint8_t status = 0;
	struct pm8x41_gpio gpio;

	/* CDP vol_up seems to be always grounded. So gpio status is read as 0,
	 * whether key is pressed or not.
	 * Ignore volume_up key on CDP for now.
	 */
	if (board_hardware_id() == HW_PLATFORM_SURF)
		return 0;

	/* Configure the GPIO */
	gpio.direction = PM_GPIO_DIR_IN;
	gpio.function  = 0;
	gpio.pull      = PM_GPIO_PULL_UP_30;
	gpio.vin_sel   = 2;

	pm8x41_gpio_config(5, &gpio);

	/* Wait for the pmic gpio config to take effect */
	thread_sleep(1);

	/* Get status of P_GPIO_5 */
	pm8x41_gpio_get(5, &status);

	return !status; /* active low */
}

#if WITH_LK2ND
uint32_t target_volume_down_old()
#else
/* Return 1 if vol_down pressed */
uint32_t target_volume_down()
#endif
{
	/* Volume down button is tied in with RESIN on MSM8974. */
	if (platform_is_8974() && (pmic_ver == PM8X41_VERSION_V2))
		return pm8x41_v2_resin_status();
	else
		return pm8x41_resin_status();
}

#if WITH_LK2ND
extern void target_keystatus();
#else
static void target_keystatus()
{
	keys_init();

	if(target_volume_down())
		keys_post_event(KEY_VOLUMEDOWN, 1);

	if(target_volume_up())
		keys_post_event(KEY_VOLUMEUP, 1);
}
#endif

/* Set up params for h/w CE. */
void target_crypto_init_params()
{
	struct crypto_init_params ce_params;

	/* Set up base addresses and instance. */
	ce_params.crypto_instance  = CE_INSTANCE;
	ce_params.crypto_base      = MSM_CE2_BASE;
	ce_params.bam_base         = MSM_CE2_BAM_BASE;

	/* Set up BAM config. */
	ce_params.bam_ee               = CE_EE;
	ce_params.pipes.read_pipe      = CE_READ_PIPE;
	ce_params.pipes.write_pipe     = CE_WRITE_PIPE;
	ce_params.pipes.read_pipe_grp  = CE_READ_PIPE_LOCK_GRP;
	ce_params.pipes.write_pipe_grp = CE_WRITE_PIPE_LOCK_GRP;

	/* Assign buffer sizes. */
	ce_params.num_ce           = CE_ARRAY_SIZE;
	ce_params.read_fifo_size   = CE_FIFO_SIZE;
	ce_params.write_fifo_size  = CE_FIFO_SIZE;

	/* BAM is initialized by TZ for this platform.
	 * Do not do it again as the initialization address space
	 * is locked.
	 */
	ce_params.do_bam_init      = 0;

	crypto_init_params(&ce_params);
}

crypto_engine_type board_ce_type(void)
{
	return CRYPTO_ENGINE_TYPE_HW;
}

#if MMC_SDHCI_SUPPORT
static void target_mmc_sdhci_init()
{
	struct mmc_config_data config = {0};
	uint32_t soc_ver = 0;

	soc_ver = board_soc_version();

	/*
	 * 8974 v1 fluid devices, have a hardware bug
	 * which limits the bus width to 4 bit.
	 */
	switch(board_hardware_id())
	{
		case HW_PLATFORM_FLUID:
			if (platform_is_8974() && BOARD_SOC_VERSION1(soc_ver))
				config.bus_width = DATA_BUS_WIDTH_4BIT;
			else
				config.bus_width = DATA_BUS_WIDTH_8BIT;
			break;
		default:
			config.bus_width = DATA_BUS_WIDTH_8BIT;
	};

	/* Trying Slot 1*/
	config.slot = 1;
	/*
	 * For 8974 AC platform the software clock
	 * plan recommends to use the following frequencies:
	 * 200 MHz --> 192 MHZ
	 * 400 MHZ --> 384 MHZ
	 * only for emmc slot
	 */
	if (platform_is_8974ac())
		config.max_clk_rate = MMC_CLK_192MHZ;
	else
		config.max_clk_rate = MMC_CLK_200MHZ;
	config.sdhc_base = mmc_sdhci_base[config.slot - 1];
	config.pwrctl_base = mmc_sdc_base[config.slot - 1];
	config.pwr_irq     = mmc_sdc_pwrctl_irq[config.slot - 1];
	config.hs400_support = 1;

	if (!(dev = mmc_init(&config))) {
		/* Trying Slot 2 next */
		config.slot = 2;
		config.max_clk_rate = MMC_CLK_200MHZ;
		config.sdhc_base = mmc_sdhci_base[config.slot - 1];
		config.pwrctl_base = mmc_sdc_base[config.slot - 1];
		config.pwr_irq     = mmc_sdc_pwrctl_irq[config.slot - 1];

		if (!(dev = mmc_init(&config))) {
			dprintf(CRITICAL, "mmc init failed!");
			ASSERT(0);
		}
	}

	/*
	 * MMC initialization is complete, read the partition table info
	 */
	if (partition_read_table()) {
		dprintf(CRITICAL, "Error reading the partition table info\n");
		ASSERT(0);
	}
}

void *target_mmc_device()
{
	return (void *) dev;
}

#else
static void target_mmc_mci_init()
{
	uint32_t base_addr;
	uint8_t slot;

	/* Trying Slot 1 */
	slot = 1;
	base_addr = mmc_sdc_base[slot - 1];

	if (mmc_boot_main(slot, base_addr))
	{
		/* Trying Slot 2 next */
		slot = 2;
		base_addr = mmc_sdc_base[slot - 1];
		if (mmc_boot_main(slot, base_addr)) {
			dprintf(CRITICAL, "mmc init failed!");
			ASSERT(0);
		}
	}
}

/*
 * Function to set the capabilities for the host
 */
void target_mmc_caps(struct mmc_host *host)
{
	uint32_t soc_ver = 0;

	soc_ver = board_soc_version();

	/*
	 * 8974 v1 fluid devices, have a hardware bug
	 * which limits the bus width to 4 bit.
	 */
	switch(board_hardware_id())
	{
		case HW_PLATFORM_FLUID:
			if (platform_is_8974() && BOARD_SOC_VERSION1(soc_ver))
				host->caps.bus_width = MMC_BOOT_BUS_WIDTH_4_BIT;
			else
				host->caps.bus_width = MMC_BOOT_BUS_WIDTH_8_BIT;
			break;
		default:
			host->caps.bus_width = MMC_BOOT_BUS_WIDTH_8_BIT;
	};

	host->caps.ddr_mode = 1;
	host->caps.hs200_mode = 1;
	host->caps.hs_clk_rate = MMC_CLK_96MHZ;
}
#endif


void target_init(void)
{
	dprintf(INFO, "target_init()\n");

	spmi_init(PMIC_ARB_CHANNEL_NUM, PMIC_ARB_OWNER_ID);

	/* Save PM8941 version info. */
	pmic_ver = pm8x41_get_pmic_rev();

	target_keystatus();

	if (target_use_signed_kernel())
		target_crypto_init_params();

	/*
	 * Set drive strength & pull ctrl for
	 * emmc
	 */
	set_sdc_power_ctrl();

#if MMC_SDHCI_SUPPORT
	target_mmc_sdhci_init();
#else
	target_mmc_mci_init();
#endif
}

unsigned board_machtype(void)
{
	return target_id;
}

/* Do any target specific intialization needed before entering fastboot mode */
#ifdef SSD_ENABLE
static void ssd_load_keystore_from_emmc()
{
	uint64_t           ptn    = 0;
	int                index  = -1;
	uint32_t           size   = SSD_PARTITION_SIZE;
	int                ret    = -1;

	uint32_t *buffer = (uint32_t *)memalign(CACHE_LINE,
								   ROUNDUP(SSD_PARTITION_SIZE, CACHE_LINE));

	if (!buffer) {
		dprintf(CRITICAL, "Error Allocating memory for SSD buffer\n");
		ASSERT(0);
	}

	index = partition_get_index("ssd");

	ptn   = partition_get_offset(index);
	if(ptn == 0){
		dprintf(CRITICAL,"ERROR: ssd parition not found");
		return;
	}

	if(mmc_read(ptn, buffer, size)){
		dprintf(CRITICAL,"ERROR:Cannot read data\n");
		return;
	}

	ret = scm_protect_keystore((uint32_t *)&buffer[0],size);
	if(ret != 0)
		dprintf(CRITICAL,"ERROR: scm_protect_keystore Failed");

	free(buffer);
}
#endif

void target_fastboot_init(void)
{
	/* Set the BOOT_DONE flag in PM8921 */
	pm8x41_set_boot_done();

#ifdef SSD_ENABLE
	clock_ce_enable(SSD_CE_INSTANCE_1);
	ssd_load_keystore_from_emmc();
#endif
}

/* Initialize target specific USB handlers */
target_usb_iface_t* target_usb30_init()
{
	target_usb_iface_t *t_usb_iface;

	t_usb_iface = calloc(1, sizeof(target_usb_iface_t));
	ASSERT(t_usb_iface);

	t_usb_iface->mux_config = target_usb_phy_mux_configure;
	t_usb_iface->clock_init = clock_usb30_init;

	return t_usb_iface;
}

/* Detect the target type */
void target_detect(struct board_data *board)
{
	/* This property is filled in board.c */
}

/* Detect the modem type */
void target_baseband_detect(struct board_data *board)
{
	uint32_t platform;
	uint32_t platform_subtype;

	platform = board->platform;
	platform_subtype = board->platform_subtype;

	/*
	 * Look for platform subtype if present, else
	 * check for platform type to decide on the
	 * baseband type
	 */
	switch(platform_subtype) {
	case HW_PLATFORM_SUBTYPE_UNKNOWN:
	case HW_PLATFORM_SUBTYPE_8974PRO_PM8084:
	case HW_PLATFORM_8994_INTERPOSER:
		break;
	default:
		dprintf(CRITICAL, "Platform Subtype : %u is not supported\n",platform_subtype);
		ASSERT(0);
	};

	switch(platform) {
	case MSM8974:
	case MSM8274:
	case MSM8674:
	case MSM8274AA:
	case MSM8274AB:
	case MSM8274AC:
	case MSM8674AA:
	case MSM8674AB:
	case MSM8674AC:
	case MSM8974AA:
	case MSM8974AB:
	case MSM8974AC:
		board->baseband = BASEBAND_MSM;
		break;
	case APQ8074:
	case APQ8074AA:
	case APQ8074AB:
	case APQ8074AC:
		board->baseband = BASEBAND_APQ;
		break;
	default:
		dprintf(CRITICAL, "Platform type: %u is not supported\n",platform);
		ASSERT(0);
	};
}

unsigned target_baseband()
{
	return board_baseband();
}

void target_serialno(unsigned char *buf)
{
	unsigned int serialno;
	if (target_is_emmc_boot()) {
		serialno = mmc_get_psn();
		snprintf((char *)buf, 13, "%x", serialno);
	}
}

unsigned check_reboot_mode(void)
{
	uint32_t restart_reason = 0;
	uint32_t soc_ver = 0;
	uint32_t restart_reason_addr;

	soc_ver = board_soc_version();

	if (platform_is_8974() && BOARD_SOC_VERSION1(soc_ver))
		restart_reason_addr = RESTART_REASON_ADDR;
	else
		restart_reason_addr = RESTART_REASON_ADDR_V2;

	/* Read reboot reason and scrub it */
	restart_reason = readl(restart_reason_addr);
	writel(0x00, restart_reason_addr);

	return restart_reason;
}

void reboot_device(unsigned reboot_reason)
{
	uint32_t soc_ver = 0;
	uint8_t reset_type = 0;

	soc_ver = board_soc_version();

	/* Write the reboot reason */
	if (platform_is_8974() && BOARD_SOC_VERSION1(soc_ver))
		writel(reboot_reason, RESTART_REASON_ADDR);
	else
		writel(reboot_reason, RESTART_REASON_ADDR_V2);

	if(reboot_reason == FASTBOOT_MODE)
		reset_type = PON_PSHOLD_WARM_RESET;
	else
		reset_type = PON_PSHOLD_HARD_RESET;

	/* Configure PMIC for warm reset */
	if (platform_is_8974() && (pmic_ver == PM8X41_VERSION_V2))
		pm8x41_v2_reset_configure(reset_type);
	else
		pm8x41_reset_configure(reset_type);

	/* Drop PS_HOLD for MSM */
	writel(0x00, MPM2_MPM_PS_HOLD);

	mdelay(5000);

	dprintf(CRITICAL, "Rebooting failed\n");
}

int set_download_mode(enum dload_mode mode)
{
	dload_util_write_cookie(mode == NORMAL_DLOAD ?
		DLOAD_MODE_ADDR_V2 : EMERGENCY_DLOAD_MODE_ADDR_V2, mode);

	return 0;
}

/* Check if MSM needs VBUS mimic for USB */
static int target_needs_vbus_mimic()
{
	if (platform_is_8974())
		return 0;

	return 1;
}

/* Do target specific usb initialization */
void target_usb_init(void)
{
	uint32_t val;

	/* Enable secondary USB PHY on DragonBoard8074 */
	if (board_hardware_id() == HW_PLATFORM_DRAGON) {
		/* Route ChipIDea to use secondary USB HS port2 */
		writel_relaxed(1, USB2_PHY_SEL);

		/* Enable access to secondary PHY by clamping the low
		* voltage interface between DVDD of the PHY and Vddcx
		* (set bit16 (USB2_PHY_HS2_DIG_CLAMP_N_2) = 1) */
		writel_relaxed(readl_relaxed(USB_OTG_HS_PHY_SEC_CTRL)
				| 0x00010000, USB_OTG_HS_PHY_SEC_CTRL);

		/* Perform power-on-reset of the PHY.
		*  Delay values are arbitrary */
		writel_relaxed(readl_relaxed(USB_OTG_HS_PHY_CTRL)|1,
				USB_OTG_HS_PHY_CTRL);
		thread_sleep(10);
		writel_relaxed(readl_relaxed(USB_OTG_HS_PHY_CTRL) & 0xFFFFFFFE,
				USB_OTG_HS_PHY_CTRL);
		thread_sleep(10);

		/* Enable HSUSB PHY port for ULPI interface,
		* then configure related parameters within the PHY */
		writel_relaxed(((readl_relaxed(USB_PORTSC) & 0xC0000000)
				| 0x8c000004), USB_PORTSC);
	}

	if (target_needs_vbus_mimic())
	{
		/* Select and enable external configuration with USB PHY */
		ulpi_write(ULPI_MISC_A_VBUSVLDEXTSEL | ULPI_MISC_A_VBUSVLDEXT, ULPI_MISC_A_SET);

		/* Enable sess_vld */
		val = readl(USB_GENCONFIG_2) | GEN2_SESS_VLD_CTRL_EN;
		writel(val, USB_GENCONFIG_2);

		/* Enable external vbus configuration in the LINK */
		val = readl(USB_USBCMD);
		val |= SESS_VLD_CTRL;
		writel(val, USB_USBCMD);
	}
}

uint8_t target_panel_auto_detect_enabled()
{
	switch(board_hardware_id())
	{
		case HW_PLATFORM_SURF:
		case HW_PLATFORM_MTP:
		case HW_PLATFORM_FLUID:
			return 1;
			break;
		default:
			return 0;
			break;
	}
	return 0;
}

uint8_t target_is_edp()
{
	switch(board_hardware_id())
	{
		case HW_PLATFORM_LIQUID:
			return 1;
			break;
		default:
			return 0;
			break;
	}
	return 0;
}

static uint8_t splash_override;
/* Returns 1 if target supports continuous splash screen. */
int target_cont_splash_screen()
{
	uint8_t splash_screen = 0;
	if(!splash_override) {
		switch(board_hardware_id())
		{
			case HW_PLATFORM_SURF:
			case HW_PLATFORM_MTP:
			case HW_PLATFORM_FLUID:
			case HW_PLATFORM_DRAGON:
			case HW_PLATFORM_LIQUID:
				dprintf(SPEW, "Target_cont_splash=1\n");
				splash_screen = 1;
				break;
			default:
				dprintf(SPEW, "Target_cont_splash=0\n");
				splash_screen = 0;
		}
	}
	return splash_screen;
}

void target_force_cont_splash_disable(uint8_t override)
{
	splash_override = override;
}

unsigned target_pause_for_battery_charge(void)
{
	uint8_t pon_reason = pm8x41_get_pon_reason();

        /* This function will always return 0 to facilitate
         * automated testing/reboot with usb connected.
         * uncomment if this feature is needed */
	/* if ((pon_reason == USB_CHG) || (pon_reason == DC_CHG))
		return 1;*/

	return 0;
}

void target_uninit(void)
{
#if MMC_SDHCI_SUPPORT
	mmc_put_card_to_sleep(dev);
#else
	mmc_put_card_to_sleep();
#endif
#ifdef SSD_ENABLE
	clock_ce_disable(SSD_CE_INSTANCE_1);
#endif
	if (crypto_initialized())
		crypto_eng_cleanup();

	/* Disable HC mode before jumping to kernel */
	sdhci_mode_disable(&dev->host);
}

void shutdown_device()
{
	dprintf(CRITICAL, "Going down for shutdown.\n");

	/* Configure PMIC for shutdown. */
	if (platform_is_8974() && (pmic_ver == PM8X41_VERSION_V2))
		pm8x41_v2_reset_configure(PON_PSHOLD_SHUTDOWN);
	else
		pm8x41_reset_configure(PON_PSHOLD_SHUTDOWN);

	/* Drop PS_HOLD for MSM */
	writel(0x00, MPM2_MPM_PS_HOLD);

	mdelay(5000);

	dprintf(CRITICAL, "Shutdown failed\n");
}

static void set_sdc_power_ctrl()
{
	uint8_t tlmm_hdrv_clk = 0;
	uint32_t platform_id = 0;

	platform_id = board_platform_id();

	switch(platform_id)
	{
		case MSM8274AA:
		case MSM8274AB:
		case MSM8674AA:
		case MSM8674AB:
		case MSM8974AA:
		case MSM8974AB:
			if (board_hardware_id() == HW_PLATFORM_MTP)
				tlmm_hdrv_clk = TLMM_CUR_VAL_10MA;
			else
				tlmm_hdrv_clk = TLMM_CUR_VAL_16MA;
			break;
		default:
			tlmm_hdrv_clk = TLMM_CUR_VAL_16MA;
	};

	/* Drive strength configs for sdc pins */
	struct tlmm_cfgs sdc1_hdrv_cfg[] =
	{
		{ SDC1_CLK_HDRV_CTL_OFF,  tlmm_hdrv_clk, TLMM_HDRV_MASK },
		{ SDC1_CMD_HDRV_CTL_OFF,  TLMM_CUR_VAL_10MA, TLMM_HDRV_MASK },
		{ SDC1_DATA_HDRV_CTL_OFF, TLMM_CUR_VAL_10MA, TLMM_HDRV_MASK },
	};

	/* Pull configs for sdc pins */
	struct tlmm_cfgs sdc1_pull_cfg[] =
	{
		{ SDC1_CLK_PULL_CTL_OFF,  TLMM_NO_PULL, TLMM_PULL_MASK },
		{ SDC1_CMD_PULL_CTL_OFF,  TLMM_PULL_UP, TLMM_PULL_MASK },
		{ SDC1_DATA_PULL_CTL_OFF, TLMM_PULL_UP, TLMM_PULL_MASK },
	};

	struct tlmm_cfgs sdc1_rclk_cfg[] =
	{
		{ SDC1_RCLK_PULL_CTL_OFF, TLMM_PULL_DOWN, TLMM_PULL_MASK },
	};

	/* Set the drive strength & pull control values */
	tlmm_set_hdrive_ctrl(sdc1_hdrv_cfg, ARRAY_SIZE(sdc1_hdrv_cfg));
	tlmm_set_pull_ctrl(sdc1_pull_cfg, ARRAY_SIZE(sdc1_pull_cfg));

	/* RCLK is supported only with 8974 pro, set rclk to pull down
	 * only for 8974 pro targets
	 */
	if (!platform_is_8974())
		tlmm_set_pull_ctrl(sdc1_rclk_cfg, ARRAY_SIZE(sdc1_rclk_cfg));
}

int emmc_recovery_init(void)
{
	return _emmc_recovery_init();
}

void target_usb_stop(void)
{
	uint32_t platform = board_platform_id();

	/* Disable VBUS mimicing in the controller. */
	if (target_needs_vbus_mimic())
		ulpi_write(ULPI_MISC_A_VBUSVLDEXTSEL | ULPI_MISC_A_VBUSVLDEXT, ULPI_MISC_A_CLEAR);
}

/* identify the usb controller to be used for the target */
const char * target_usb_controller()
{
	switch(board_platform_id())
	{
		/* use dwc controller for PRO chips (with some exceptions) */
		case MSM8974AA:
		case MSM8974AB:
		case MSM8974AC:
			/* exceptions based on hardware id */
			if (board_hardware_id() != HW_PLATFORM_DRAGON && !target_hw_interposer())
				return "dwc";
		/* fall through to default "ci" for anything that did'nt select "dwc" */
		default:
			return "ci";
	}
}

/* UTMI MUX configuration to connect PHY to SNPS controller:
 * Configure primary HS phy mux to use UTMI interface
 * (connected to usb30 controller).
 */
static void tcsr_hs_phy_mux_configure(void)
{
	uint32_t reg;

	reg = readl(USB2_PHY_SEL);

	writel(reg | 0x1, USB2_PHY_SEL);
}

/* configure hs phy mux if using dwc controller */
void target_usb_phy_mux_configure(void)
{
	if(!strcmp(target_usb_controller(), "dwc"))
	{
		tcsr_hs_phy_mux_configure();
	}
}

static struct spmi_regulator pm8941_regulators[] = {
	{"s1", 0x11400},
	{"s2", 0x11700},
	{"s3", 0x11a00},
	{"s4", 0x1a000},

	{"l1", 0x14000},
	{"l2", 0x14100},
	{"l3", 0x14200},
	{"l4", 0x14300},
	{"l5", 0x14400},
	{"l6", 0x14500},
	{"l7", 0x14600},
	{"l8", 0x14700},
	{"l9", 0x14800},
	{"l10", 0x14900},
	{"l11", 0x14a00},
	{"l12", 0x14b00},
	{"l13", 0x14c00},
	{"l14", 0x14d00},
	{"l15", 0x14e00},
	{"l16", 0x14f00},
	{"l17", 0x15000},
	{"l18", 0x15100},
	{"l19", 0x15200},
	{"l20", 0x15300},
	{"l21", 0x15400},
	{"l22", 0x15500},
	{"l23", 0x15600},
	{"l24", 0x15700},

	{"lvs1", 0x18000},
	{"lvs2", 0x18100},
	{"lvs3", 0x18200},

	{"5vs1", 0x18300},
	{"5vs2", 0x18400},
	{ }
};

struct spmi_regulator* target_get_regulators()
{
	regulators_init(pm8941_regulators);
	return pm8941_regulators;
}
