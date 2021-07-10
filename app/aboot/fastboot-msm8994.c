#include <reg.h>
#include <stdio.h>
#include "fastboot.h"

#define QFPROM_BASE		0xfc4b8000
#define SPEEDBIN_EFUSE	QFPROM_BASE + 0xB0
#define JTAG_EFUSE		QFPROM_BASE + 0x6024

#define OEM_CONFIG0		(0x000)
#define OEM_CONFIG1		(0x004)
#define OEM_CONFIG2		(0x008)

/* JTAG/CORESIGHT FUSE V1 */
#define BIT(n) (1UL) << (n)
#define ALL_DEBUG_DISABLE		BIT(21)
#define APPS_DBGEN_DISABLE      BIT(0)
#define APPS_NIDEN_DISABLE      BIT(1)
#define APPS_SPIDEN_DISABLE     BIT(2)
#define APPS_SPNIDEN_DISABLE    BIT(3)
#define DAP_DBGEN_DISABLE       BIT(4)
#define DAP_NIDEN_DISABLE       BIT(5)
#define DAP_SPIDEN_DISABLE      BIT(6)
#define DAP_SPNIDEN_DISABLE     BIT(7)
#define DAP_DEVICEEN_DISABLE    BIT(8)

#define A53OFFSET		56

static void cmd_oem_dump_fuses(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	uint32_t efuse = readl(SPEEDBIN_EFUSE);
	uint32_t jtag_efuse = readl(JTAG_EFUSE);
	uint32_t jtag_config0 = readl(jtag_efuse + OEM_CONFIG0);
	uint32_t jtag_config1 = readl(jtag_efuse + OEM_CONFIG1);
	int bin, a53speedbin;

	/* 8994 being 8994, is of course weird.. A53 bin is always 0 */
	bin = efuse & 0x7;
	snprintf(response, sizeof(response), "A57 Speed bin: %d", bin);
	fastboot_info(response);

	/*
	// 8994 always seems to have A53 bin 0
	if(8992) {
		a53speedbin = (efuse >> A53OFFSET) & 0x3;
		snprintf(response, sizeof(response), "A53 Speed bin: %d", a53speedbin);
		fastboot_info(response);
	}
	*/
	snprintf(response, sizeof(response), "\n--- JTAG/CORESIGHT ---");
	fastboot_info(response);

	snprintf(response, sizeof(response), "ALL_DEBUG_DISABLE: 0x%X", jtag_config0 & ALL_DEBUG_DISABLE);
	fastboot_info(response);

	snprintf(response, sizeof(response), "APPS_DBGEN_DISABLE: 0x%X", jtag_config1 & APPS_DBGEN_DISABLE);
	fastboot_info(response);

	snprintf(response, sizeof(response), "APPS_NIDEN_DISABLE: 0x%X", jtag_config1 & APPS_NIDEN_DISABLE);
	fastboot_info(response);

	snprintf(response, sizeof(response), "APPS_SPIDEN_DISABLE: 0x%X", jtag_config1 & APPS_SPIDEN_DISABLE);
	fastboot_info(response);

	snprintf(response, sizeof(response), "APPS_SPNIDEN_DISABLE: 0x%X", jtag_config1 & APPS_SPNIDEN_DISABLE);
	fastboot_info(response);

	snprintf(response, sizeof(response), "DAP_DEVICEEN_DISABLE: 0x%X", jtag_config1 & DAP_DEVICEEN_DISABLE);
	fastboot_info(response);

	fastboot_okay("");
}

void target_fastboot_register_commands(void) {
	fastboot_register("oem dump-fuses", cmd_oem_dump_fuses);
}
