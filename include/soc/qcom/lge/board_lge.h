#ifndef __ASM_ARCH_MSM_BOARD_LGE_H
#define __ASM_ARCH_MSM_BOARD_LGE_H

#if defined(CONFIG_LGE_LCD_MFTS_MODE)
extern int lge_get_mfts_mode(void);
#endif

#if defined(CONFIG_PRE_SELF_DIAGNOSIS)
int lge_pre_self_diagnosis(char *drv_bus_code, int func_code, char *dev_code, char *drv_code, int errno);
#endif
#if defined(CONFIG_PRE_SELF_DIAGNOSIS)
struct pre_selfd_platform_data {
	int (*set_values) (int r, int g, int b);
	int (*get_values) (int *r, int *g, int *b);
};
#endif
#ifdef CONFIG_LGE_USB_FACTORY
typedef enum {
	LGE_BOOT_MODE_NORMAL = 0,
	LGE_BOOT_MODE_CHARGER,
	LGE_BOOT_MODE_CHARGERLOGO,
	LGE_BOOT_MODE_QEM_56K,
	LGE_BOOT_MODE_QEM_130K,
	LGE_BOOT_MODE_QEM_910K,
	LGE_BOOT_MODE_PIF_56K,
	LGE_BOOT_MODE_PIF_130K,
	LGE_BOOT_MODE_PIF_910K,
	LGE_BOOT_MODE_MINIOS    /* LGE_UPDATE for MINIOS2.0 */
} lge_boot_mode_t;

typedef enum {
	LGE_FACTORY_CABLE_NONE = 0,
	LGE_FACTORY_CABLE_56K,
	LGE_FACTORY_CABLE_130K,
	LGE_FACTORY_CABLE_910K,
} lge_factory_cable_t;

typedef enum {
	LGE_LAF_MODE_NORMAL = 0,
	LGE_LAF_MODE_LAF,
} lge_laf_mode_t;

lge_boot_mode_t lge_get_boot_mode(void);
bool lge_get_factory_boot(void);
lge_factory_cable_t lge_get_factory_cable(void);
bool lge_get_android_dlcomplete(void);
lge_laf_mode_t lge_get_laf_mode(void);
#endif

#if defined(CONFIG_LGE_PM_LGE_POWER_CLASS_CABLE_DETECT)
#include <soc/qcom/lge/power/lge_cable_detect.h>
cable_boot_type lge_get_boot_cable(void);
#endif

extern int lge_get_bootreason(void);

#ifdef CONFIG_LGE_LCD_OFF_DIMMING
extern int lge_get_bootreason_with_lcd_dimming(void);
#endif

#ifdef CONFIG_MACH_LGE
bool lge_check_recoveryboot(void);

typedef enum {
	LGE_HYDRA_MODE_ALPHA = 0,
	LGE_HYDRA_MODE_PRIME,
	LGE_HYDRA_MODE_PLUS,
	LGE_HYDRA_MODE_SIGNATURE,
	LGE_HYDRA_MODE_RENEWAL,
	LGE_HYDRA_MODE_NONE,
} lge_hydra_mode_t;
int lge_get_hydra_mode(void);

typedef enum {
	NA_ALL= 0,
	GLOBAL,
	NONE,
} lge_sku_carrier_t;
int lge_get_sku_carrier(void);
#endif

#if defined(CONFIG_LGE_DISPLAY_COMMON)
int lge_get_panel(void);
void lge_set_panel(int);
int lge_get_panel_flag_status(void);
int lge_get_uefi_panel_status(void);
#endif

#if defined(CONFIG_LGE_DISPLAY_COMMON)
int lge_get_lk_panel_status(void);
int lge_get_dsv_status(void);
int lge_get_panel(void);
void lge_set_panel(int);
#endif

#if defined(CONFIG_LGE_PANEL_MAKER_ID_SUPPORT)
enum panel_maker_id_type {
	LGD_LG4946 = 0,
	LGD_LG4945,
	LGD_S3320,
	LGD_TD4302,
	PANEL_MAKER_ID_MAX
};

enum panel_maker_id_type lge_get_panel_maker_id(void);
#endif

#if defined(CONFIG_LGE_DISPLAY_COMMON)
enum panel_revision_id_type {
	LGD_LG4946_REV0 = 0,
	LGD_LG4946_REV1,
	LGD_LG4946_REV2,
	LGD_LG4946_REV3,
	PANEL_REVISION_ID_MAX
};

enum panel_revision_id_type lge_get_panel_revision_id(void);
#endif

#ifdef CONFIG_LGE_LCD_TUNING
struct lcd_platform_data {
int (*set_values) (int *tun_lcd_t);
int (*get_values) (int *tun_lcd_t);
};

void __init lge_add_lcd_misc_devices(void);
#endif

#endif /* __ASM_ARCH_MSM_BOARD_LGE_H */
