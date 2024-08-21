/*
 * Copyright(c) 2016, LG Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt)	"[Display] %s: " fmt, __func__

#include <linux/of_platform.h>
#include "../mdss_dsi.h"
#include "lge_mdss_dsi_panel.h"
#include <linux/delay.h>

#if IS_ENABLED(CONFIG_LGE_DISPLAY_READER_MODE)
#include "lge_reader_mode.h"
#include "../mdss_mdp.h"
struct dsi_panel_cmds reader_mode_step0_cmds;
struct dsi_panel_cmds reader_mode_step1_cmds;
struct dsi_panel_cmds reader_mode_step2_cmds;
struct dsi_panel_cmds reader_mode_step3_cmds;
struct dsi_panel_cmds reader_mode_step4_cmds;
struct dsi_panel_cmds reader_mode_step5_cmds;
struct dsi_panel_cmds reader_mode_step6_cmds;
struct dsi_panel_cmds reader_mode_step7_cmds;
struct dsi_panel_cmds reader_mode_step8_cmds;
struct dsi_panel_cmds reader_mode_step9_cmds;
struct dsi_panel_cmds reader_mode_step10_cmds;
#endif

#if defined(CONFIG_LGE_DISPLAY_CONTROL)
#include "lge_display_control.h"
#if defined(CONFIG_LGE_DISPLAY_COMFORT_MODE)
#include "lge_comfort_view.h"
#endif
#endif
#if defined(CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED)
#include "lge_mdss_ambient.h"
#endif /* CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED */

struct mdss_panel_data *pdata_base;

char *lge_blmap_name[] = {
	"lge,blmap",
#if defined(CONFIG_LGE_HIGH_LUMINANCE_MODE)
	"lge,blmap-hl",
#endif
#if defined(CONFIG_LGE_DISPLAY_CONTROL)
#if defined(CONFIG_LGE_DISPLAY_VIDEO_ENHANCEMENT)
	"lge,blmap-ve",
#endif /* CONFIG_LGE_DISPLAY_VIDEO_ENHANCEMENT */
#endif /* CONFIG_LGE_DISPLAY_CONTROL */
#if defined(CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED)
	"lge,blmap-ex",
#if defined(LGE_DISPLAY_BRIGHTNESS_DIMMING)
	"lge,blmap-ex-dim",
#endif
#if defined(CONFIG_LGE_HIGH_LUMINANCE_MODE)
	"lge,blmap-ex-hl",
#if defined(LGE_DISPLAY_BRIGHTNESS_DIMMING)
	"lge,blmap-ex-dim-hl",
#endif
#endif
#endif /* CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED */
};

#if defined(CONFIG_LGE_DISPLAY_COMMON)
extern int panel_not_connected;
int detect_factory_cable(void);
#endif /* CONFIG_LGE_DISPLAY_COMMON*/

#if defined(CONFIG_LGE_LCD_TUNING)
extern int tun_lcd[128];
extern char read_cmd[128];
extern int reg_num;
int cmd_num;
#endif

extern int mdss_dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key);
extern void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
		struct dsi_panel_cmds *pcmds, u32 flags);

void lge_mdss_dsi_parse_dcs_cmds_by_name_array(struct device_node *np,
		struct lge_dsi_cmds_entry **lge_dsi_cmds_list,
		char *cmd_name_array[],
		int num_cmds)
{
	int i, rc = 0;
	char *name;
	char cmd_state[128];
	struct lge_dsi_cmds_entry *cmds_list = NULL;

	if (np == NULL || num_cmds == 0 || cmd_name_array == NULL) {
		pr_err("Invalid input\n");
		return;
	}

	if (*lge_dsi_cmds_list == NULL) {
		cmds_list = kzalloc(sizeof(struct lge_dsi_cmds_entry) *	num_cmds, GFP_KERNEL);
		*lge_dsi_cmds_list = cmds_list;
	} else {
		pr_err("This cmd list is already allocated.\n");
		return;
	}

	if (cmds_list == NULL) {
		pr_err("no memory\n");
		return;
	}

	for (i = 0; i < num_cmds; ++i) {
		name = cmd_name_array[i];
		strlcpy(cmds_list[i].name, name,
						sizeof(cmds_list[i].name));
		strlcpy(cmd_state, name, sizeof(cmd_state));
		strcat(cmd_state, "-state");
		pr_info("name : %s, cmd state : %s\n", name, cmd_state);
		rc = mdss_dsi_parse_dcs_cmds(np,	&cmds_list[i].lge_dsi_cmds, name, cmd_state);
		if (!rc)
			pr_info("lge_dsi_cmds_list[%d].lge_dsi_cmds : 0x%02x\n", i, cmds_list[i].lge_dsi_cmds.cmds[0].payload[0]);
	}
}

void lge_mdss_dsi_send_dcs_cmds_list(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
				struct lge_dsi_cmds_entry *lge_dsi_cmds_list, int num_cmds)
{
	int i;
	for (i = 0; i < num_cmds ; i++) {
		if (lge_dsi_cmds_list[i].lge_dsi_cmds.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl_pdata,
							&lge_dsi_cmds_list[i].lge_dsi_cmds,
							CMD_REQ_COMMIT);
	}
}

void lge_mdss_dsi_send_dcs_cmds_by_cmd_name(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
				struct lge_dsi_cmds_entry *lge_dsi_cmds_list, int num_cmds, const char *cmd_name)
{
	int i, index = -1;
	for (i = 0; i < num_cmds; ++i) {
		if (!strcmp(lge_dsi_cmds_list[i].name, cmd_name)) {
			index = i;
			break;
		}
	}

	if (index != -1) {
		if (lge_dsi_cmds_list[index].lge_dsi_cmds.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl_pdata,
							&lge_dsi_cmds_list[index].lge_dsi_cmds,
							CMD_REQ_COMMIT);
	} else {
		pr_err("cmds %s not found\n", cmd_name);
	}
}

#if IS_ENABLED(CONFIG_LGE_DISPLAY_READER_MODE)
int lge_mdss_dsi_parse_reader_mode_cmds(struct device_node *np, struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step0_cmds,
			"qcom,panel-reader-mode-step0-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step1_cmds,
			"qcom,panel-reader-mode-step1-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step2_cmds,
			"qcom,panel-reader-mode-step2-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step3_cmds,
			"qcom,panel-reader-mode-step3-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step4_cmds,
			"qcom,panel-reader-mode-step4-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step5_cmds,
			"qcom,panel-reader-mode-step5-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step6_cmds,
			"qcom,panel-reader-mode-step6-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step7_cmds,
			"qcom,panel-reader-mode-step7-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step8_cmds,
			"qcom,panel-reader-mode-step8-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step9_cmds,
			"qcom,panel-reader-mode-step9-command", "qcom,mdss-dsi-reader-mode-command-state");
	mdss_dsi_parse_dcs_cmds(np, &reader_mode_step10_cmds,
			"qcom,panel-reader-mode-step10-command", "qcom,mdss-dsi-reader-mode-command-state");
	return 0;
}

bool lge_change_reader_mode(struct mdss_dsi_ctrl_pdata *ctrl, int new_mode)
{
	switch(new_mode) {
		char mask;
	case READER_MODE_STEP_1:
		pr_info("%s: Reader Mode Step 1\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step1_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_2:
		pr_info("%s: Reader Mode Step 2\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step2_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_3:
		pr_info("%s: Reader Mode Step 3\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step3_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_4:
		pr_info("%s: Reader Mode Step 4\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step4_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_5:
		pr_info("%s: Reader Mode Step 5\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step5_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_6:
		pr_info("%s: Reader Mode Step 6\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step6_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_7:
		pr_info("%s: Reader Mode Step 7\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step7_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_8:
		pr_info("%s: Reader Mode Step 8\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step8_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_9:
		pr_info("%s: Reader Mode Step 9\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step9_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;
	case READER_MODE_STEP_10:
		pr_info("%s: Reader Mode Step 10\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step10_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		ctrl->reg_f0h_cmds.cmds[0].payload[1] |= READER_GC_MASK;
#endif
		break;

	case READER_MODE_OFF:
	default:
		pr_info("%s: Reader Mode Step OFF\n",__func__);
		mdss_dsi_panel_cmds_send(ctrl, &reader_mode_step0_cmds, CMD_REQ_COMMIT);
		mask = MONO_MASK;
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
		ctrl->reg_55h_cmds.cmds[0].payload[1] &= (~mask);
		mask = READER_GC_MASK;
		ctrl->reg_f0h_cmds.cmds[0].payload[1] &= (~mask);
#endif
	}
#if defined(CONFIG_LGE_DISPLAY_LUCYE_COMMON)
	mdss_dsi_panel_cmds_send(ctrl, &ctrl->reg_55h_cmds, CMD_REQ_COMMIT);
	mdss_dsi_panel_cmds_send(ctrl, &ctrl->reg_f0h_cmds, CMD_REQ_COMMIT);
	pr_info("%s : 55h:0x%02x, f0h:0x%02x, f2h(SH):0x%02x, fbh(CABC):0x%02x \n",__func__,
		ctrl->reg_55h_cmds.cmds[0].payload[1],	ctrl->reg_f0h_cmds.cmds[0].payload[1],
		ctrl->reg_f2h_cmds.cmds[0].payload[3], ctrl->reg_fbh_cmds.cmds[0].payload[4]);
#endif
	return true;
}

int lge_mdss_dsi_panel_send_on_cmds(struct mdss_dsi_ctrl_pdata *ctrl, struct dsi_panel_cmds *default_on_cmds, int cur_mode)
{
	if (default_on_cmds->cmd_cnt)
		mdss_dsi_panel_cmds_send(ctrl, default_on_cmds, CMD_REQ_COMMIT);

	lge_change_reader_mode(ctrl, cur_mode);

	return 0;
}
#endif

#if defined(CONFIG_LGE_DISPLAY_BIST_MODE)
int lge_mdss_dsi_bist_ctrl(struct mdss_dsi_ctrl_pdata *ctrl, bool enable)
{
	if (enable) {
		if (ctrl->bist_on == 0) {
			mdss_dsi_panel_cmds_send(ctrl,
				&ctrl->bist_on_cmds,
				CMD_REQ_COMMIT);
		}
		ctrl->bist_on++;
	} else {
		if (ctrl->bist_on == 1) {
			mdss_dsi_panel_cmds_send(ctrl,
				&ctrl->bist_off_cmds,
				CMD_REQ_COMMIT);
		}
		ctrl->bist_on--;
		if (ctrl->bist_on < 0) {
			pr_err("count (%d) -> (0) : debugging!\n", ctrl->bist_on);
			ctrl->bist_on = 0;
		}
	}
	pr_info("count(%d)\n", ctrl->bist_on);
	return 0;
}

void lge_mdss_dsi_bist_release(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl->bist_on > 0) {
		ctrl->bist_on = 1;
		if (lge_mdss_dsi_bist_ctrl(ctrl, false) < 0) {
			pr_warn("fail to bist control\n");
		}
	}
	pr_info("%d\n", ctrl->bist_on);
}
#endif

#ifdef CONFIG_ARCH_MSM8996
char lge_dcs_cmd[1] = {0xD8}; /* DTYPE_DCS_READ */
#else
char lge_dcs_cmd[2] = {0x00, 0x00}; /* DTYPE_DCS_READ */
#endif
struct dsi_cmd_desc lge_dcs_read_cmd = {
	{DTYPE_DCS_READ, 1, 0, 1, 1, sizeof(lge_dcs_cmd)},
	lge_dcs_cmd
};

int lge_mdss_dsi_panel_cmd_read(char cmd0, int cnt, char* ret_buf)
{
	struct dcs_cmd_req cmdreq;
	struct mdss_dsi_ctrl_pdata *ctrl;
	char rx_buf[128] = {0x0};
	int i = 0;
	int checksum = 0;
	int ret = 0;

	if(panel_not_connected) {
		pr_err("Skip Panel Cmd Read : Panel not connected.\n");
		return -EINVAL;
	}

	if (pdata_base == NULL) {
		pr_err("Invalid input data\n");
		return -EINVAL;
	}

	ctrl = container_of(pdata_base, struct mdss_dsi_ctrl_pdata,
						panel_data);
	lge_dcs_cmd[0] = cmd0;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &lge_dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = cnt;
	cmdreq.cb = NULL;
	cmdreq.rbuf = rx_buf;

	if (ctrl->status_cmds.link_state == DSI_LP_MODE)
		cmdreq.flags |= CMD_REQ_LP_MODE;
	else if (ctrl->status_cmds.link_state == DSI_HS_MODE)
		cmdreq.flags |= CMD_REQ_HS_MODE;

	mdelay(1);

	if(!mdss_dsi_cmdlist_put(ctrl, &cmdreq)) {
		pr_err("Panel Command Read Failed!\n");
		return -EAGAIN;
	}

	for (i = 0; i < cnt; i++)
		checksum += rx_buf[i];

	if (checksum == 0) {
		pr_err("[Reg:0x%x] All data is zero\n", cmd0);
	} else {
		pr_info("[Reg:0x%x] checksum (%d)\n", cmd0, checksum);
		for (i = 0; i < cnt; i++) {
			pr_debug("Reg[0x%x], buf[%d]=0x%x\n", cmd0, i, rx_buf[i]);
		}
	}

	memcpy(ret_buf, rx_buf, cnt);

	return ret;
}

int lge_is_valid_U2_FTRIM_reg(void)
{
	int ret = 0;
	char ret_buf[4] = {0x0};
	char cmd_addr[1] = {0xC7};

	if(pdata_base->panel_info.panel_power_state == 0){
		pr_err("%s: Cannot read U2 FTRIM reg because panel is off state.\n", __func__);
		return -ENODEV;
	}

	lge_mdss_dsi_panel_cmd_read(cmd_addr[0], 4, ret_buf);

	if (ret_buf[3] & BIT(6)) {
		ret = 1;
	}

	return ret;
}
EXPORT_SYMBOL(lge_is_valid_U2_FTRIM_reg);

#if defined(CONFIG_LGE_DISPLAY_MFTS_DET_SUPPORTED) && !defined(CONFIG_LGE_DISPLAY_DYN_DSI_MODE_SWITCH)
static int mfts_video_cnt;
int lge_set_validate_lcd_reg(void)
{
	int i = 0;
	int ret = 0;
	int cnt = 13;
	char ret_buf[13] = {0x0};
	char cmd_addr[1] = {0xC7};
	struct mdss_dsi_ctrl_pdata *ctrl;

	ctrl = container_of(pdata_base, struct mdss_dsi_ctrl_pdata,
						panel_data);
	if(pdata_base->panel_info.panel_power_state == 0){
		pr_err("%s: Cannot check TRIM reg because panel is off state.\n", __func__);
		return -ENODEV;
	}

	lge_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

	if(pdata_base->panel_info.is_validate_lcd == 1) //for MFTS mode
	{
		memcpy(&(ctrl->screen_cmds_102v.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg before writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->screen_cmds_102v.cmds[0].payload[i]);
		}
		pr_info("\n");

		//1.02V
		ctrl->screen_cmds_102v.cmds[0].payload[9] = 0x00;

		mdss_dsi_panel_cmds_send(ctrl, &ctrl->screen_cmds_102v, CMD_REQ_COMMIT);
		lge_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

		memcpy(&(ctrl->screen_cmds_102v.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg after writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->screen_cmds_102v.cmds[0].payload[i]);
		}
		pr_info("\n");
	} else if(pdata_base->panel_info.is_validate_lcd == 2) { //for MFTS mode
		memcpy(&(ctrl->screen_cmds_129v.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg before writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->screen_cmds_129v.cmds[0].payload[i]);
		}
		pr_info("\n");

		//1.29V
		ctrl->screen_cmds_129v.cmds[0].payload[9] += 0x22;

		mdss_dsi_panel_cmds_send(ctrl, &ctrl->screen_cmds_129v, CMD_REQ_COMMIT);
		lge_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

		memcpy(&(ctrl->screen_cmds_129v.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg after writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->screen_cmds_129v.cmds[0].payload[i]);
		}
		pr_info("\n");
	}  else if(pdata_base->panel_info.is_validate_lcd == 3) { //for MFTS mode
		memcpy(&(ctrl->screen_cmds_132v.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg before writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->screen_cmds_132v.cmds[0].payload[i]);
		}
		pr_info("\n");
		switch(mfts_video_cnt){
			case 0:
				ctrl->cam_cmds.cmds[0].payload[1] = 0x99;
				mdss_dsi_panel_cmds_send(ctrl, &ctrl->cam_cmds, CMD_REQ_COMMIT);
				pr_info("mfts_video_cnt : %d\n", mfts_video_cnt);
				mfts_video_cnt++;
				break;
			case 1:
				ctrl->cam_cmds.cmds[0].payload[1] = 0x9a;
				mdss_dsi_panel_cmds_send(ctrl, &ctrl->cam_cmds, CMD_REQ_COMMIT);
				pr_info("mfts_video_cnt : %d\n", mfts_video_cnt);
				mfts_video_cnt++;
				break;
			case 2:
				ctrl->cam_cmds.cmds[0].payload[1] = 0x9b;
				mdss_dsi_panel_cmds_send(ctrl, &ctrl->cam_cmds, CMD_REQ_COMMIT);
				pr_info("mfts_video_cnt : %d\n", mfts_video_cnt);
				break;
			default:
				break;
		}
		//1.32V
		ctrl->screen_cmds_132v.cmds[0].payload[9] += 0x33;
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->screen_cmds_132v, CMD_REQ_COMMIT);
		lge_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

		memcpy(&(ctrl->screen_cmds_132v.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg after writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->screen_cmds_132v.cmds[0].payload[i]);
		}
		pr_info("\n");
	} else {
		memcpy(&(ctrl->trimming_cmds.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg before writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->trimming_cmds.cmds[0].payload[i]);
		}
		pr_info("\n");

		/* We don't need to set vdds to 1.29V anymore if this video test isn't working.
		//vdds 1.29V
		ctrl->trimming_cmds.cmds[0].payload[9] += 0x22;
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->trimming_cmds, CMD_REQ_COMMIT);
		*/
		lge_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

		memcpy(&(ctrl->trimming_cmds.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg after writing mode %d : ", pdata_base->panel_info.is_validate_lcd);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->trimming_cmds.cmds[0].payload[i]);
		}
		mfts_video_cnt = 0;
		pr_info("\n");
	}

	return ret;
}
EXPORT_SYMBOL(lge_set_validate_lcd_reg);

int lge_set_validate_lcd_cam(int mode)
{
	int i = 0;
	int ret = 0;
	int cnt = 13;
	char ret_buf[13] = {0x0};
	char cmd_addr[1] = {0xC7};
	struct mdss_dsi_ctrl_pdata *ctrl;

	ctrl = container_of(pdata_base, struct mdss_dsi_ctrl_pdata,
						panel_data);
	if(pdata_base->panel_info.panel_power_state == 0){
		pr_err("%s: Cannot check TRIM reg because panel is off state.\n", __func__);
		return -ENODEV;
	}

	lge_force_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

	if(mode == 1) //for AAT cam Test
	{
		memcpy(&(ctrl->trimming_cmds.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg before writing mode %d : ", mode);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->trimming_cmds.cmds[0].payload[i]);
		}
		pr_info("\n");

		//vdds 1.32V
		ctrl->trimming_cmds.cmds[0].payload[9] += 0x11;
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->trimming_cmds, CMD_REQ_COMMIT);

		//cam cmds
		ctrl->cam_cmds.cmds[0].payload[1] = 0x9A;
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->cam_cmds, CMD_REQ_COMMIT);

		pr_info("cam reg writing mode : ");
		for ( i = 0; i < 7 + 1; i++) {
			pr_info("0x%x ", ctrl->cam_cmds.cmds[0].payload[i]);
		}
		pr_info("\n");

		lge_force_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

		memcpy(&(ctrl->trimming_cmds.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg after writing mode %d : ", mode);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->trimming_cmds.cmds[0].payload[i]);
		}
		pr_info("\n");
	} else { // recovery
		memcpy(&(ctrl->trimming_cmds.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg before writing mode %d : ", mode);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->trimming_cmds.cmds[0].payload[i]);
		}
		pr_info("\n");

		//vdds 1.32V
		ctrl->trimming_cmds.cmds[0].payload[9] -= 0x11;
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->trimming_cmds, CMD_REQ_COMMIT);

		//cam time
		ctrl->cam_cmds.cmds[0].payload[1] = 0x98;
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->cam_cmds, CMD_REQ_COMMIT);

		pr_info("hold time reg writing mode : ");
		for ( i = 0; i < 7 + 1; i++) {
			pr_info("0x%x ", ctrl->cam_cmds.cmds[0].payload[i]);
		}
		pr_info("\n");

		lge_force_mdss_dsi_panel_cmd_read(cmd_addr[0], cnt, ret_buf);

		memcpy(&(ctrl->trimming_cmds.cmds[0].payload[1]), ret_buf, cnt);

		pr_info("trim reg after writing mode %d : ", mode);
		for ( i = 0; i < cnt + 1; i++) {
			pr_info("0x%x ", ctrl->trimming_cmds.cmds[0].payload[i]);
		}
		pr_info("\n");
	}
	return ret;
}
EXPORT_SYMBOL(lge_set_validate_lcd_cam);
#endif

#if defined(CONFIG_LGE_DISPLAY_AOD_WITH_MIPI)
char lge_dcs_cmd_watch[1] = {0xD8}; /* DTYPE_DCS_READ */
struct dsi_cmd_desc lge_dcs_read_cmd_watch = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(lge_dcs_cmd_watch)},
	lge_dcs_cmd_watch,
};
void lge_watch_mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl,
		char cmd0, int cnt, char* ret_buf)
{
	struct dcs_cmd_req cmdreq;
	char rx_buf[128] = {0x0};

	lge_dcs_cmd_watch[0] = cmd0;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &lge_dcs_read_cmd_watch;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = cnt;
	cmdreq.cb = NULL;
	cmdreq.rbuf = rx_buf;

	if (ctrl->status_cmds.link_state == DSI_LP_MODE)
		cmdreq.flags |= CMD_REQ_LP_MODE;
	else if (ctrl->status_cmds.link_state == DSI_HS_MODE)
		cmdreq.flags |= CMD_REQ_HS_MODE;

	mdelay(1);

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);

	//for(i=0;i<cnt;i++)
	//	pr_info("[Display]Reg[0x%x],buf[%d]=0x%x,mode[%d]\n",cmd0, i, rx_buf[i], ctrl->status_cmds.link_state);

	memcpy(ret_buf, rx_buf, cnt);
}
#endif

#if defined(CONFIG_LGE_LCD_TUNING)
int find_lcd_cmd(void)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	int i,j;
	char cmd[128]={0, };
	memset(read_cmd,0,128*sizeof(char));
	pr_info("reg_num=%x",reg_num);
	if(pdata_base == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl =  container_of(pdata_base, struct mdss_dsi_ctrl_pdata,
			panel_data);
	for(i=0;i<ctrl->on_cmds.cmd_cnt;i++)
	{
		pr_info("%s:cmd_cnt(find_lcd_cmd)[%d]=%x",__func__,i,ctrl->on_cmds.cmds[i].payload[0]);
		if(ctrl->on_cmds.cmds[i].payload[0]==reg_num)
		{
			for(j=0;j<ctrl->on_cmds.cmds[i].dchdr.dlen;j++)
			{
				cmd[j] = ctrl->on_cmds.cmds[i].payload[j];
			}
			memcpy(read_cmd,cmd,128*sizeof(char));
			cmd_num=ctrl->on_cmds.cmds[i].dchdr.dlen;
			return cmd_num;
		}
	}
	return 0;
}
void put_lcd_cmd(void)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	int i,j;
	pr_info("reg_num=%x",reg_num);
	if(pdata_base == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl =  container_of(pdata_base, struct mdss_dsi_ctrl_pdata,
			panel_data);
	for(i=0;i<ctrl->on_cmds.cmd_cnt;i++)
	{
		pr_info("%s:cmd_cnt[%d]=%x",__func__,i,ctrl->on_cmds.cmds[i].payload[0]);
		if(ctrl->on_cmds.cmds[i].payload[0]==reg_num)
		{
			for(j=1;j<ctrl->on_cmds.cmds[i].dchdr.dlen;j++)
			{
				ctrl->on_cmds.cmds[i].payload[j]=tun_lcd[j];
				pr_info("%s: cmds[%d].payload[%d]: %x",__func__,i,j,ctrl->on_cmds.cmds[i].payload[j]);
			}
			//mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds, CMD_REQ_COMMIT);
		}
	}
}

int get_backlight_map_size(int *bl_size)
{
	int ret = 0;
#if defined(CONFIG_LGE_DISPLAY_COMMON)
	if (pdata_base) {
		*bl_size = pdata_base->panel_info.blmap_size;
		pr_info("Current bl map size : %d\n", *bl_size);
	}
#else
	ret = -1;
#endif
	return ret;
}

int get_backlight_map(int *bl_map)
{
	int ret = 0;
#if defined(CONFIG_LGE_DISPLAY_COMMON)
	if (pdata_base) {
		memcpy(bl_map, pdata_base->panel_info.blmap, sizeof(int) * pdata_base->panel_info.blmap_size);
	}
#else
	ret = -1;
#endif
	return ret;
}
int set_backlight_map(int bl_size, int *bl_map)
{
	int ret = 0;
#if defined(CONFIG_LGE_DISPLAY_COMMON)
	if (pdata_base) {
		pdata_base->panel_info.blmap_size = bl_size;
		memcpy(pdata_base->panel_info.blmap, bl_map, sizeof(int) * bl_size);
		pr_info("BL map updated with size %d\n", bl_size);
	}
#else
	ret = -1;
#endif
	return ret;
}

#endif

char *lge_get_blmapname(enum lge_bl_map_type  blmaptype)
{
	if (blmaptype >= 0 && blmaptype < LGE_BLMAPMAX)
		return lge_blmap_name[blmaptype];
	else
		return lge_blmap_name[LGE_BLDFT];
}

void lge_mdss_panel_parse_dt_blmaps(struct device_node *np,
				   struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int i, j, rc;
	u32 *array;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);

	pinfo->blmap_size = 256;
	array = kzalloc(sizeof(u32) * pinfo->blmap_size, GFP_KERNEL);

	if (!array)
		return;

	for (i = 0; i < LGE_BLMAPMAX; i++) {
		/* check if property exists */
		if (!of_find_property(np, lge_blmap_name[i], NULL))
			continue;

		pr_info("found %s\n", lge_blmap_name[i]);

		rc = of_property_read_u32_array(np, lge_blmap_name[i], array,
						pinfo->blmap_size);
		if (rc) {
			pr_err("%d, unable to read %s\n", __LINE__, lge_blmap_name[i]);
			goto error;
		}

		pinfo->blmap[i] = kzalloc(sizeof(int) * pinfo->blmap_size,
				GFP_KERNEL);

		if (!pinfo->blmap[i]){
			goto error;
		}

		for (j = 0; j < pinfo->blmap_size; j++)
			pinfo->blmap[i][j] = array[j];
	}

	kfree(array);
	return;

error:
	for (i = 0; i < LGE_BLMAPMAX; i++)
		if (pinfo->blmap[i])
			kfree(pinfo->blmap[i]);
	kfree(array);
}

static int lge_mdss_dsi_panel_create_sysfs(struct lge_mdss_dsi_ctrl_pdata *lge_ctrl_pdata)
{
	int rc = 0;
	static struct class *panel = NULL;
	if (!panel) {
		panel = class_create(THIS_MODULE, "panel");
		if (IS_ERR(panel)) {
			pr_err("Failed to create panel class\n");
			return -EINVAL;
		}
 	}

	if (lge_ctrl_pdata != NULL && lge_ctrl_pdata->create_panel_sysfs) {
		rc = lge_ctrl_pdata->create_panel_sysfs(panel);
		if (rc < 0)
			pr_err("Panel-dependent sysfs creation failed\n");
	}

#if defined(CONFIG_LGE_DISPLAY_CONTROL)
	if ((rc = lge_display_control_create_sysfs(panel)) < 0) {
		pr_err("fail to create display control sysfs\n");
 	}
#endif /* CONFIG_LGE_DISPLAY_CONTROL */
#if defined(CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED)
	if ((rc = lge_mdss_ambient_create_sysfs(panel)) < 0) {
		pr_err("fail to create ambient sysfs\n");
 	}
#endif /* CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED */
	return rc;
}

static int lge_mdss_panel_parse_dt(struct device_node *np,
						struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct lge_mdss_dsi_ctrl_pdata *lge_ctrl_pdata = NULL;

	if (np == NULL || ctrl_pdata == NULL) {
		pr_err("Invalid input\n");
		return -EINVAL;
	}

	lge_ctrl_pdata = ctrl_pdata->lge_ctrl_pdata;

	if (lge_ctrl_pdata == NULL) {
		pr_err("Invalid pdata state\n");
		return -EINVAL;
	}

	if (lge_ctrl_pdata->parse_dt_blmaps)
		lge_ctrl_pdata->parse_dt_blmaps(np, ctrl_pdata);
	else
		lge_mdss_panel_parse_dt_blmaps(np, ctrl_pdata);

#if IS_ENABLED(CONFIG_LGE_DISPLAY_READER_MODE)
	lge_mdss_dsi_parse_reader_mode_cmds(np, ctrl_pdata);
#endif

#if defined(CONFIG_LGE_DISPLAY_CONTROL)
	mdss_dsi_parse_display_control_dcs_cmds(np, ctrl_pdata);
#endif /* CONFIG_LGE_DISPLAY_CONTROL*/
#if defined(CONFIG_LGE_DISPLAY_BIST_MODE)
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->bist_on_cmds,
			"lge,bist-on-cmds", "qcom,mode-control-dsi-state");
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->bist_off_cmds,
			"lge,bist-off-cmds", "qcom,mode-control-dsi-state");
#endif
	return 0;
}

int lge_mdss_dsi_panel_init(struct device_node *node, struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	struct lge_mdss_dsi_ctrl_pdata *lge_ctrl_pdata = NULL;

#if defined(CONFIG_LGE_LCD_DYNAMIC_CABC_MIE_CTRL)
	ctrl_pdata->ie_on = 1;
#endif
#if defined(CONFIG_LGE_DISPLAY_AOD_SUPPORTED)
	if ((rc = oem_mdss_aod_init(node, ctrl_pdata)))
		return rc;
#endif

	if (ctrl_pdata->panel_data.panel_info.pdest == DISPLAY_1) {
		if (pdata_base == NULL)
			pdata_base = &(ctrl_pdata->panel_data);
	}

	lge_ctrl_pdata = kzalloc(sizeof(struct lge_mdss_dsi_ctrl_pdata), GFP_KERNEL);
	if (!lge_ctrl_pdata) {
		pr_err("Unable to alloc mem for lge_crtl_pdata\n");
		rc = -ENOMEM;
		goto mem_fail;
	}

	pr_err("ctrl_pdata = %p, lge_ctrl_pdata = %p\n", ctrl_pdata, lge_ctrl_pdata);
	lge_ctrl_pdata->lge_blmap_list = lge_blmap_name;
	ctrl_pdata->lge_ctrl_pdata = lge_ctrl_pdata;

	/* This funciton should be defined under each model directory */
	lge_mdss_dsi_panel_init_sub(lge_ctrl_pdata);

#if defined(CONFIG_LGE_DISPLAY_CONTROL)
	rc = lge_display_control_init(ctrl_pdata);
	if (rc) {
		pr_err("fail to init display control (rc:%d)\n", rc);
		return rc;
	}
#endif /* CONFIG_LGE_DISPLAY_CONTROL */

#if defined(CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED)
	rc = lge_mdss_ambient_init(node, ctrl_pdata);
	if (rc) {
		pr_err("[Ambient] fail to init (rc:%d)\n", rc);
		return rc;
	}
#endif

	lge_mdss_panel_parse_dt(node, ctrl_pdata);
	rc = lge_mdss_dsi_panel_create_sysfs(lge_ctrl_pdata);
mem_fail:
	return rc;
}

int lge_mdss_dsi_panel_reg_backup(struct mdss_dsi_ctrl_pdata *ctrl) {
	int ret = 0;
	int ret_sub = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct lge_mdss_dsi_ctrl_pdata *lge_ctrl_pdata = NULL;

	if (pdata_base == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata_base, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (ctrl_pdata == NULL) {
		pr_err("Invalid input\n");
		return -EINVAL;
	}

	lge_ctrl_pdata = ctrl_pdata->lge_ctrl_pdata;

	if (lge_ctrl_pdata == NULL) {
		pr_err("Invalid pdata state\n");
		return -EINVAL;
	}

#if defined(CONFIG_LGE_DISPLAY_BRIGHTNESS_DIMMING)
	if (!lge_ctrl_pdata->bc_reg_backup_flag) {
		ret_sub = lge_bc_dim_reg_backup(ctrl_pdata);
		if (ret_sub < 0) {
			pr_err("fail to backup BC CTRL REG : %d\n", ret_sub);
		} else {
			lge_ctrl_pdata->bc_reg_backup_flag = true;
			lge_bc_dim_set(ctrl_pdata, BC_DIM_ON, BC_DIM_FRAMES_NORMAL);
		}
		ret += ret_sub;
	}
#endif

#if defined(CONFIG_LGE_DISPLAY_COLOR_MANAGER)
	if (!lge_ctrl_pdata->is_backup) {
		ret_sub = lge_color_manager_reg_backup(ctrl_pdata);
		if (ret_sub < 0)
			pr_err("fail to backup CM REG : %d\n", ret_sub);
		else
			lge_ctrl_pdata->is_backup = true;
		ret += ret_sub;
	}
#endif

#if defined(CONFIG_LGE_DISPLAY_VR_MODE)
	if (!lge_ctrl_pdata->vr_reg_backup) {
		ret_sub = lge_vr_low_persist_reg_backup(ctrl_pdata);
		if (ret_sub < 0)
			pr_err("fail to backup VR Persist REG : %d\n", ret_sub);
		else
			lge_ctrl_pdata->vr_reg_backup = true;
		ret += ret_sub;
	}
#endif

#if defined(CONFIG_LGE_DISPLAY_AMBIENT_SUPPORTED)
	if (!ctrl_pdata->ambient_reg_backup) {
		ret_sub = lge_mdss_ambient_backup_internal_reg(ctrl_pdata);
		if (ret_sub < 0)
			pr_err("fail to backup Ambient REG : %d\n", ret_sub);
		else
			ctrl_pdata->ambient_reg_backup = true;
		ret += ret_sub;
	}
#endif

	return ret;
}
