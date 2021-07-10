/* Copyright (c) 2015-2017, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*	notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*	copyright notice, this list of conditions and the following
*	disclaimer in the documentation and/or other materials provided
*	with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*	contributors may be used to endorse or promote products derived
*	from this software without specific prior written permission.
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
#include <reg.h>
#include <stdlib.h>
#include <dev/fbcon.h>
#include <kernel/thread.h>
#include <display_menu.h>
#include <menu_keys_detect.h>
#include <boot_verifier.h>
#include <string.h>
#include <platform.h>
#include <smem.h>
#include <target.h>
#include <sys/types.h>
#include <../../../app/aboot/devinfo.h>
#include <lk2nd.h>
#if TARGET_MSM8916 || TARGET_MSM8994
#include "psci.h"
#endif

static const char *unlock_menu_common_msg = "If you unlock the bootloader, "\
				"you will be able to install "\
				"custom operating system on this phone.\n\n"\
				"A custom OS is not subject to the same testing "\
				"as the original OS, "\
				"and can cause your phone and installed "\
				"applications to stop working properly.\n\n"\
				"To prevent unauthorized access to your personal data, "\
				"unlocking the bootloader will also delete all personal "\
				"data from your phone(a \"fatory data reset\").\n\n"\
				"Press the Volume Up/Down buttons to select Yes "\
				"or No. Then press the Power button to continue.\n";

#define YELLOW_WARNING_MSG	"Your device has loaded a different operating system\n\n "\
				"Visit this link on another device:\n g.co/ABH"

#define ORANGE_WARNING_MSG	"Your device software can't be\n checked for corruption. Please lock the bootloader\n\n"\
				"Visit this link on another device:\n g.co/ABH"

#define RED_WARNING_MSG	"Your device is corrupt. It can't be\ntrusted and will not boot\n\n" \
				"Visit this link on another device:\n g.co/ABH"

#define LOGGING_WARNING_MSG	"The dm-verity is not started in enforcing mode and may "\
				"not work properly\n\nTo learn more, visit:\n"

#define EIO_WARNING_MSG		"Your device is corrupt. It can't be\n trusted and may not work properly.\n\n"\
				"Visit this link on another device:\n g.co/ABH"

#define DELAY_5SEC 5000
#define DELAY_30SEC 30000

static bool is_thread_start = false;
static struct select_msg_info msg_info;

#if VERIFIED_BOOT
struct boot_verify_info {
	int msg_type;
	const char *warning_msg;
};

struct boot_verify_info boot_verify_info[] = {
			[DISPLAY_MENU_RED] = {FBCON_RED_MSG, RED_WARNING_MSG},
			[DISPLAY_MENU_YELLOW] = {FBCON_YELLOW_MSG, YELLOW_WARNING_MSG},
			[DISPLAY_MENU_ORANGE] = {FBCON_ORANGE_MSG, ORANGE_WARNING_MSG},
			[DISPLAY_MENU_LOGGING] = {FBCON_RED_MSG, LOGGING_WARNING_MSG},
			[DISPLAY_MENU_EIO] = {FBCON_RED_MSG, EIO_WARNING_MSG}};
#endif

static char *verify_option_menu[] = {
		[POWEROFF] = "Power off\n",
		[RESTART] = "Restart\n",
		[RECOVER] = "Recovery\n",
		[FASTBOOT] = "Fastboot\n",
		[BACK] = "Back to previous page\n"};

static char *fastboot_option_menu[] = {
		[0] = "START\n",
		[1] = "Restart bootloader\n",
		[2] = "Recovery mode\n",
		[3] = "Power off\n"};

static int big_factor = 2;
static int common_factor = 1;

static void wait_for_exit()
{
	struct select_msg_info *select_msg;
	select_msg = &msg_info;

	mutex_acquire(&select_msg->msg_lock);
	while(!select_msg->info.rel_exit == true) {
		mutex_release(&select_msg->msg_lock);
		thread_sleep(10);
		mutex_acquire(&select_msg->msg_lock);
	}
	mutex_release(&select_msg->msg_lock);

	is_thread_start = false;
	fbcon_clear();
	display_image_on_screen();
}

void wait_for_users_action()
{
	/* Waiting for exit menu keys detection if there is no any usr action
	 * otherwise it will do the action base on the keys detection thread
	 */
	wait_for_exit();
}

void exit_menu_keys_detection()
{
	struct select_msg_info *select_msg;
	select_msg = &msg_info;

	mutex_acquire(&select_msg->msg_lock);
	select_msg->info.is_exit = true;
	mutex_release(&select_msg->msg_lock);

	wait_for_exit();
}

static void set_message_factor()
{
	uint32_t tmp_factor = 0;
	uint32_t max_x_count = 0;
	uint32_t max_x = 0;

	if (fbcon_get_width() < fbcon_get_height())
		max_x_count = CHAR_NUM_PERROW_POR;
	else
		max_x_count = CHAR_NUM_PERROW_HOR;

	max_x = fbcon_get_max_x();
	tmp_factor = max_x/max_x_count;

	if(tmp_factor <= 1) {
		big_factor = 2;
		common_factor = 1;
	} else {
		big_factor = tmp_factor*2;
		common_factor = tmp_factor;
	}
}

static void display_fbcon_menu_message(char *str, unsigned type,
	unsigned scale_factor)
{
	while(*str != 0) {
		fbcon_putc_factor(*str++, type, scale_factor);
	}
}

static char *str_align_right(char *str, int factor)
{
	uint32_t max_x = 0;
	int diff = 0;
	int i = 0;
	char *str_target = NULL;

	max_x = fbcon_get_max_x();
	if (!str_target && max_x) {
		str_target = malloc(max_x);
	}

	if (str_target) {
		memset(str_target, 0, max_x);
		if ( max_x/factor > strlen(str)) {
			if (factor == 1)
				diff = max_x/factor - strlen(str) - 1;
			else
				diff = max_x/factor - strlen(str);
			for (i = 0; i < diff; i++) {
				strcat(str_target, " ");
			}
			strcat(str_target, str);
			return str_target;
		} else {
			free(str_target);
			return str;
		}
	}
	return str;
}

/* msg_lock need to be holded when call this function. */
void display_unlock_menu_renew(struct select_msg_info *unlock_msg_info, int type)
{
	fbcon_clear();
	memset(&unlock_msg_info->info, 0, sizeof(struct menu_info));

	display_fbcon_menu_message("Unlock bootloader?\n",
		FBCON_UNLOCK_TITLE_MSG, big_factor);
	fbcon_draw_line(FBCON_COMMON_MSG);

	display_fbcon_menu_message((char*)unlock_menu_common_msg,
		FBCON_COMMON_MSG, common_factor);
	fbcon_draw_line(FBCON_COMMON_MSG);
	unlock_msg_info->info.option_start[0] = fbcon_get_current_line();
	display_fbcon_menu_message("Yes\n",
		FBCON_COMMON_MSG, big_factor);
	unlock_msg_info->info.option_bg[0] = fbcon_get_current_bg();
	display_fbcon_menu_message("Unlock bootloader(may void warranty)\n",
		FBCON_COMMON_MSG, common_factor);
	unlock_msg_info->info.option_end[0] = fbcon_get_current_line();
	fbcon_draw_line(FBCON_COMMON_MSG);
	unlock_msg_info->info.option_start[1] = fbcon_get_current_line();
	display_fbcon_menu_message("No\n",
		FBCON_COMMON_MSG, big_factor);
	unlock_msg_info->info.option_bg[1] = fbcon_get_current_bg();
	display_fbcon_menu_message("Do not unlock bootloader and restart phone\n",
		FBCON_COMMON_MSG, common_factor);
	unlock_msg_info->info.option_end[1] = fbcon_get_current_line();
	fbcon_draw_line(FBCON_COMMON_MSG);

	if (type == UNLOCK)
		unlock_msg_info->info.msg_type = DISPLAY_MENU_UNLOCK;
	else if (type == UNLOCK_CRITICAL)
		unlock_msg_info->info.msg_type = DISPLAY_MENU_UNLOCK_CRITICAL;

	unlock_msg_info->info.option_num = 2;

	/* Initialize the option index */
	unlock_msg_info->info.option_index= 2;
}

#if VERIFIED_BOOT
/* msg_lock need to be holded when call this function. */
void display_bootverify_menu_renew(struct select_msg_info *msg_info, int type)
{
	unsigned char* fp_buf = NULL;
	char fp_str_temp[EVP_MAX_MD_SIZE] = {'\0'};
	char fp_str[EVP_MAX_MD_SIZE*2] = {'\0'};
	char str_temp[8];

	char str1[]= "Start >";
	char str2[] = "Continue boot";
	char *str3 = NULL;

	char *str_target = NULL;
	uint32 fp_size = 0;
	unsigned int i = 0;
	uint32_t timeout = DELAY_5SEC;

	fbcon_clear();
	memset(&msg_info->info, 0, sizeof(struct menu_info));

	/* Align Right */
	str_target = str_align_right(str1, big_factor);
	if(str_target != NULL)
		display_fbcon_menu_message(str_target, FBCON_TITLE_MSG, big_factor);

	str_target = str_align_right(str2, common_factor);
	if(str_target != NULL)
		display_fbcon_menu_message(str_target, FBCON_TITLE_MSG, common_factor);

	display_fbcon_menu_message("\n< More options\n",
		FBCON_COMMON_MSG, common_factor);
	display_fbcon_menu_message("press VOLUME keys\n\n",
		FBCON_SUBTITLE_MSG, common_factor);
	if(boot_verify_info[type].warning_msg != NULL)
		display_fbcon_menu_message((char*)boot_verify_info[type].warning_msg,
			FBCON_COMMON_MSG, common_factor);

	display_fbcon_menu_message("g.co/placeholder\n",
		boot_verify_info[type].msg_type, common_factor);

	if (type == DISPLAY_MENU_YELLOW) {
		fp_buf = get_boot_fingerprint(&fp_size);
		if (fp_buf != NULL) {
			strncpy(fp_str_temp, (char const *)fp_buf, fp_size);
			for (i = 0; i < fp_size; i++) {
				if(i == fp_size - 1)
					sprintf(str_temp, "%02x", fp_str_temp[i]);
				else
					sprintf(str_temp, "%02x-", fp_str_temp[i]);

				strcat(fp_str, str_temp);
			}
		}
		display_fbcon_menu_message("ID:", FBCON_COMMON_MSG, common_factor);
		display_fbcon_menu_message(fp_str, FBCON_COMMON_MSG, common_factor);
	}

	str3 = "\n\nIf no key pressed:\n"\
		"Your device will boot in 5  seconds\n\n";
#if ENABLE_VB_ATTEST
	if(type == DISPLAY_MENU_EIO)
	{
		str3 ="\n\nIf power key is not pressed:\n"\
			"Your device will poweroff in 30 seconds\n\n";
		timeout = DELAY_30SEC;
	}
#endif
	display_fbcon_menu_message(str3, FBCON_COMMON_MSG, common_factor);

	msg_info->info.msg_type = type;
	if(str_target) {
		free(str_target);
	}

	/* Initialize the time out time */
	msg_info->info.timeout_time = timeout; //5s
}
#endif

/* msg_lock need to be holded when call this function. */
void display_bootverify_option_menu_renew(struct select_msg_info *msg_info)
{
	int i = 0;
	int len = 0;

	fbcon_clear();
	memset(&msg_info->info, 0, sizeof(struct menu_info));

	len = ARRAY_SIZE(verify_option_menu);
	display_fbcon_menu_message("Options menu:\n\n",
		FBCON_COMMON_MSG, big_factor);
	display_fbcon_menu_message("Press volume keys to navigate, and "\
		"press power key to select\n\n", FBCON_COMMON_MSG, common_factor);

	for (i = 0; i < len; i++) {
		fbcon_draw_line(FBCON_COMMON_MSG);
		msg_info->info.option_start[i] = fbcon_get_current_line();
		display_fbcon_menu_message(verify_option_menu[i],
			FBCON_COMMON_MSG, common_factor);
		msg_info->info.option_bg[i]= fbcon_get_current_bg();
		msg_info->info.option_end[i]= fbcon_get_current_line();
	}

	fbcon_draw_line(FBCON_COMMON_MSG);
	msg_info->info.msg_type = DISPLAY_MENU_MORE_OPTION;
	msg_info->info.option_num = len;

	/* Initialize the option index */
	msg_info->info.option_index= len;
}

static void display_fastboot_menu_print_fw_info(char *msg, size_t msg_size)
{
#if TARGET_MSM8916 || TARGET_MSM8994
	bool armv8 = is_scm_armv8_support();
	scmcall_arg arg = {0};
	uint32_t psci_version = PSCI_RET_NOT_SUPPORTED;
	bool hvc_el2 = false;

	snprintf(msg, msg_size, "\nARM64 - %s\n",
		 armv8 ? "available" : "unavailable");
	display_fbcon_menu_message(msg, armv8 ? FBCON_GREEN_MSG : FBCON_RED_MSG,
				   common_factor);

	if (armv8) {
		/* Check PSCI version */
		arg.x0 = PSCI_0_2_FN_PSCI_VERSION;
		psci_version = scm_call2(&arg, NULL);

		/*
		 * If hypervisor responds to HVC call with -5 it means that the
		 * command was forwarded to TZ and therefore EL2 is possible.
		 * Qualcomm's original firmware usually responds with -2.
		 */
		arg.x0 = MAKE_SIP_SCM_CMD(SCM_SVC_MILESTONE_32_64_ID, SCM_SVC_MILESTONE_CMD_ID);
		arg.hvc = true;
		hvc_el2 = scm_call2(&arg, NULL) == -5;
	}

	snprintf(msg, msg_size, "EL2 - %s\n",
		 hvc_el2 ? "available" : "unavailable");
	display_fbcon_menu_message(msg, hvc_el2 ? FBCON_GREEN_MSG : FBCON_YELLOW_MSG,
				   common_factor);

	if (psci_version != PSCI_RET_NOT_SUPPORTED) {
		snprintf(msg, msg_size, "PSCI - v%d.%d detected\n",
			 PSCI_VERSION_MAJOR(psci_version), PSCI_VERSION_MINOR(psci_version));
		display_fbcon_menu_message(msg, FBCON_GREEN_MSG, common_factor);
	} else {
		display_fbcon_menu_message("PSCI - unavailable\n",
					   FBCON_YELLOW_MSG, common_factor);
	}
#endif
}

/* msg_lock need to be holded when call this function. */
void display_fastboot_menu_renew(struct select_msg_info *fastboot_msg_info)
{
	int len;
	int msg_type = FBCON_COMMON_MSG;
	char msg_buf[64];
	char msg[128];
	uint32_t res;

	/* The fastboot menu is switched base on the option index
	 * So it's need to store the index for the menu switching
	 */
	uint32_t option_index = fastboot_msg_info->info.option_index;

	fbcon_clear();
	memset(&fastboot_msg_info->info, 0, sizeof(struct menu_info));

	len = ARRAY_SIZE(fastboot_option_menu);
	switch(option_index) {
		case 0:
			msg_type = FBCON_GREEN_MSG;
			break;
		case 1:
		case 2:
			msg_type = FBCON_RED_MSG;
			break;
		case 3:
			msg_type = FBCON_COMMON_MSG;
			break;
	}
	fbcon_draw_line(msg_type);
	display_fbcon_menu_message(fastboot_option_menu[option_index],
		msg_type, big_factor);
	fbcon_draw_line(msg_type);
	display_fbcon_menu_message("\n\nPress volume keys to navigate, and "\
		"press power key to select\n\n", FBCON_COMMON_MSG, common_factor);

	display_fbcon_menu_message("FASTBOOT MODE\n", FBCON_RED_MSG, common_factor);

#ifdef LK2ND_VERSION
	snprintf(msg, sizeof(msg), "lk2nd - %s\n", LK2ND_VERSION);
	display_fbcon_menu_message(msg, FBCON_SUBTITLE_MSG, common_factor);
#endif

	get_product_name((unsigned char *) msg_buf);
	snprintf(msg, sizeof(msg), "PRODUCT_NAME - %s\n", msg_buf);
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);

#if WITH_LK2ND
	if (lk2nd_dev.model) {
		snprintf(msg, sizeof(msg), "MODEL - %s\n", lk2nd_dev.model);
		display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);
	} else {
		display_fbcon_menu_message("MODEL - Unknown (FIXME!)\n",
					   FBCON_RED_MSG, common_factor);
	}
#endif

	memset(msg_buf, 0, sizeof(msg_buf));
	smem_get_hw_platform_name((unsigned char *) msg_buf, sizeof(msg_buf));
	snprintf(msg, sizeof(msg), "VARIANT - %s %s\n",
		msg_buf, target_is_emmc_boot()? "eMMC":"UFS");
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);

	memset(msg_buf, 0, sizeof(msg_buf));
	get_bootloader_version((unsigned char *) msg_buf);
	snprintf(msg, sizeof(msg), "BOOTLOADER VERSION - %s\n",
		msg_buf);
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);

#if !WITH_LK2ND
	memset(msg_buf, 0, sizeof(msg_buf));
	get_baseband_version((unsigned char *) msg_buf);
	snprintf(msg, sizeof(msg), "BASEBAND VERSION - %s\n",
		msg_buf);
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);
#else
	if (lk2nd_dev.panel.name) {
		snprintf(msg, sizeof(msg), "PANEL - %s\n", lk2nd_dev.panel.name);
		display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);
	}
	if (lk2nd_dev.battery) {
		snprintf(msg, sizeof(msg), "BATTERY - %s\n", lk2nd_dev.battery);
		display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);
	}
#endif

	memset(msg_buf, 0, sizeof(msg_buf));
	target_serialno((unsigned char *) msg_buf);
	snprintf(msg, sizeof(msg), "SERIAL NUMBER - %s\n", msg_buf);
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor);

#if WITH_LK2ND
	if (lk2nd_dev.dt_entry.size) {
		snprintf(msg, sizeof(msg), "DTB - <%u %#x %u %#x>\n",
			 lk2nd_dev.dt_entry.platform_id,
			 lk2nd_dev.dt_entry.variant_id,
			 lk2nd_dev.dt_entry.soc_rev,
			 lk2nd_dev.dt_entry.board_hw_subtype);
		display_fbcon_menu_message(msg, lk2nd_dev.dt_entry.offset ? FBCON_YELLOW_MSG : FBCON_COMMON_MSG,
					   common_factor);
	}
#endif

	display_fastboot_menu_print_fw_info(msg, sizeof(msg));

	res = is_secure_boot_enable();
	snprintf(msg, sizeof(msg), "SECURE BOOT - %s\n",
		res ? "enabled":"disabled");
	display_fbcon_menu_message(msg, res ? FBCON_YELLOW_MSG : FBCON_GREEN_MSG,
				   common_factor);

#if !WITH_LK2ND
	snprintf(msg, sizeof(msg), "DEVICE STATE - %s\n",
		is_device_locked()? "locked":"unlocked");
	display_fbcon_menu_message(msg, FBCON_RED_MSG, common_factor);
#endif

	fastboot_msg_info->info.msg_type = DISPLAY_MENU_FASTBOOT;
	fastboot_msg_info->info.option_num = len;
	fastboot_msg_info->info.option_index = option_index;
}

void msg_lock_init()
{
	static bool is_msg_lock_init = false;
	struct select_msg_info *msg_lock_info;
	msg_lock_info = &msg_info;

	if (!is_msg_lock_init) {
		mutex_init(&msg_lock_info->msg_lock);
		is_msg_lock_init = true;
	}
}

static void display_menu_thread_start(struct select_msg_info *msg_info)
{
	thread_t *thr;

	if (!is_thread_start) {
		thr = thread_create("selectkeydetect", &select_msg_keys_detect,
			(void*)msg_info, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
		if (!thr) {
			dprintf(CRITICAL, "ERROR: creat device status detect thread failed!!\n");
			return;
		}
		thread_resume(thr);
		is_thread_start = true;
	}
}

/* The fuction be called after device in fastboot mode,
 * so it's no need to initialize the msg_lock again
 */
void display_unlock_menu(int type)
{
	struct select_msg_info *unlock_menu_msg_info;
	unlock_menu_msg_info = &msg_info;

	set_message_factor();

	msg_lock_init();
	mutex_acquire(&unlock_menu_msg_info->msg_lock);

	/* Initialize the last_msg_type */
	unlock_menu_msg_info->last_msg_type =
		unlock_menu_msg_info->info.msg_type;

	display_unlock_menu_renew(unlock_menu_msg_info, type);
	mutex_release(&unlock_menu_msg_info->msg_lock);

	dprintf(INFO, "creating unlock keys detect thread\n");
	display_menu_thread_start(unlock_menu_msg_info);
}

void display_fastboot_menu()
{
	struct select_msg_info *fastboot_menu_msg_info;
	fastboot_menu_msg_info = &msg_info;

	if (!fbcon_display())
		return;

	set_message_factor();

	msg_lock_init();
	mutex_acquire(&fastboot_menu_msg_info->msg_lock);

	/* There are 4 pages for fastboot menu:
	 * Page: Start/Fastboot/Recovery/Poweroff
	 * The menu is switched base on the option index
	 * Initialize the option index and last_msg_type
	 */
	fastboot_menu_msg_info->info.option_index = 0;
	fastboot_menu_msg_info->last_msg_type =
		fastboot_menu_msg_info->info.msg_type;

	display_fastboot_menu_renew(fastboot_menu_msg_info);
	mutex_release(&fastboot_menu_msg_info->msg_lock);

	dprintf(INFO, "creating fastboot menu keys detect thread\n");
	display_menu_thread_start(fastboot_menu_msg_info);
}

#if VERIFIED_BOOT
void display_bootverify_menu(int type)
{
	struct select_msg_info *bootverify_menu_msg_info;
	bootverify_menu_msg_info = &msg_info;

	set_message_factor();

	msg_lock_init();
	mutex_acquire(&bootverify_menu_msg_info->msg_lock);

	/* Initialize the last_msg_type */
	bootverify_menu_msg_info->last_msg_type =
		bootverify_menu_msg_info->info.msg_type;

	display_bootverify_menu_renew(bootverify_menu_msg_info, type);
	mutex_release(&bootverify_menu_msg_info->msg_lock);

	dprintf(INFO, "creating boot verify keys detect thread\n");
	display_menu_thread_start(bootverify_menu_msg_info);
}
#endif
