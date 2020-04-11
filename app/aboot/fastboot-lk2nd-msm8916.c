#include <reg.h>
#include <stdio.h>
#include <pm8x41_regulator.h>
#include "fastboot.h"

#define EFUSE1	0x0005c004
#define EFUSE	0x0005c00c

static void efuse_read_speed_bin(int *bin, int *version) {
	uint32_t pte_efuse = readl(EFUSE1);

	*version = (pte_efuse >> 18) & 0x3;
	if (!*version) {
		*bin = (pte_efuse >> 23) & 0x3;
		if (*bin)
			return;
	}

	pte_efuse = readl(EFUSE);
	*bin = (pte_efuse >> 2) & 0x7;
}

static void cmd_oem_dump_speedbin(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	int bin, version;

	efuse_read_speed_bin(&bin, &version);

	snprintf(response, sizeof(response),
		 "Speed bin: %d, PVS version: %d (qcom,speed%d-bin-v%d)",
		 bin, version, bin, version);
	fastboot_info(response);
	fastboot_okay("");
}

static void cmd_oem_dump_regulators(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	struct spmi_regulator *vreg;
	for (vreg = target_get_regulators(); vreg->name; ++vreg) {
		snprintf(response, sizeof(response), "%s: enabled: %d, voltage: %d mV",
			 vreg->name, regulator_is_enabled(vreg),
			 regulator_get_voltage(vreg));
		fastboot_info(response);
	}
	fastboot_okay("");
}

void fastboot_lk2nd_register_commands_msm8916(void) {
	fastboot_register("oem dump-speedbin", cmd_oem_dump_speedbin);
	fastboot_register("oem dump-regulators", cmd_oem_dump_regulators);
}