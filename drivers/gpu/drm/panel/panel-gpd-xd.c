/*
 * Copyright (C) 2017 Pierre-Hugues Husson <phh@phh.me>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//Helper:
//curl https://raw.githubusercontent.com/skelton/XD_444/master/kernel/arch/arm/boot/dts/lcd-wqxga-mipi.dtsi |tr -d '\n' |grep -oE 'rockchip,on-cmds[0-9]* {[^}]*}' |sed -E 's/.*rockchip,cmd = <([^>]*)>.*cmd_delay.* = <([0-9]*)>.*/\1 \2/g' |sed -E -e 's/^0x15 (0x[0-9a-f]*) (0x[0-9a-f]*) 0/mipi_dsi_dcs_write(dsi, \1 (u8[]){ \2 }, 1);/g' -e 's/^0x39 0xff 0x98 0x81 (0x[0-9a-f]*) 0/gpd_set_page\(\1\);/g'

#include <linux/backlight.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>

#include <video/mipi_display.h>

struct gpd_panel {
	struct drm_panel base;
	struct mipi_dsi_device *dsi;

	struct backlight_device *backlight;
	struct regulator *supply;
	struct gpio_desc *reset_gpio;

	bool prepared;
	bool enabled;

	const struct drm_display_mode *mode;
};

static inline struct gpd_panel *to_gpd_panel(struct drm_panel *panel)
{
	return container_of(panel, struct gpd_panel, base);
}

static int gpd_set_page(struct gpd_panel *gpd, int page)
{
	struct mipi_dsi_device *dsi = gpd->dsi;
	return mipi_dsi_dcs_write(dsi, 0xff, (u8[]){ 0x98, 0x81, page}, 3);
}

static int gpd_panel_init(struct gpd_panel *gpd)
{
	struct mipi_dsi_device *dsi = gpd->dsi;

	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	dsi->mode_flags |= MIPI_DSI_MODE_LPM;
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	//This page is NOT described in ili9881c datasheet
	gpd_set_page(gpd, 0x03);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	mipi_dsi_dcs_write(dsi, 0x01, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x02, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x03, (u8[]){ 0x53 }, 1);
	mipi_dsi_dcs_write(dsi, 0x04, (u8[]){ 0x13 }, 1);
	mipi_dsi_dcs_write(dsi, 0x05, (u8[]){ 0x13 }, 1);
	mipi_dsi_dcs_write(dsi, 0x06, (u8[]){ 0x06 }, 1);
	mipi_dsi_dcs_write(dsi, 0x07, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x08, (u8[]){ 0x04 }, 1);
	mipi_dsi_dcs_write(dsi, 0x09, (u8[]){ 0x04 }, 1);
	mipi_dsi_dcs_write(dsi, 0x0a, (u8[]){ 0x03 }, 1);
	mipi_dsi_dcs_write(dsi, 0x0b, (u8[]){ 0x03 }, 1);
	mipi_dsi_dcs_write(dsi, 0x0c, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x0d, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x0e, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x0f, (u8[]){ 0x04 }, 1);
	mipi_dsi_dcs_write(dsi, 0x10, (u8[]){ 0x04 }, 1);
	mipi_dsi_dcs_write(dsi, 0x11, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x12, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x13, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x14, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x15, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x16, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x17, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x18, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x19, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x1a, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x1b, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x1c, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x1d, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x1e, (u8[]){ 0xc0 }, 1);
	mipi_dsi_dcs_write(dsi, 0x1f, (u8[]){ 0x80 }, 1);
	mipi_dsi_dcs_write(dsi, 0x20, (u8[]){ 0x04 }, 1);
	mipi_dsi_dcs_write(dsi, 0x21, (u8[]){ 0x0b }, 1);
	mipi_dsi_dcs_write(dsi, 0x22, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x23, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x24, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x25, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x26, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x27, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x28, (u8[]){ 0x55 }, 1);
	mipi_dsi_dcs_write(dsi, 0x29, (u8[]){ 0x03 }, 1);
	mipi_dsi_dcs_write(dsi, 0x2a, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x2b, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x2c, (u8[]){ 0x06 }, 1);
	mipi_dsi_dcs_write(dsi, 0x2d, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x2e, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x2f, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x30, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x31, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x32, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x33, (u8[]){ 0x30 }, 1);
	mipi_dsi_dcs_write(dsi, 0x34, (u8[]){ 0x04 }, 1);
	mipi_dsi_dcs_write(dsi, 0x35, (u8[]){ 0x05 }, 1);
	mipi_dsi_dcs_write(dsi, 0x36, (u8[]){ 0x05 }, 1);
	mipi_dsi_dcs_write(dsi, 0x37, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x38, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x39, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x3a, (u8[]){ 0x40 }, 1);
	mipi_dsi_dcs_write(dsi, 0x3b, (u8[]){ 0x40 }, 1);
	mipi_dsi_dcs_write(dsi, 0x3c, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x3d, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x3e, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x3f, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x40, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x41, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x42, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x43, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x44, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x50, (u8[]){ 0x01 }, 1);
	mipi_dsi_dcs_write(dsi, 0x51, (u8[]){ 0x23 }, 1);
	mipi_dsi_dcs_write(dsi, 0x52, (u8[]){ 0x45 }, 1);
	mipi_dsi_dcs_write(dsi, 0x53, (u8[]){ 0x67 }, 1);
	mipi_dsi_dcs_write(dsi, 0x54, (u8[]){ 0x89 }, 1);
	mipi_dsi_dcs_write(dsi, 0x55, (u8[]){ 0xab }, 1);
	mipi_dsi_dcs_write(dsi, 0x56, (u8[]){ 0x01 }, 1);
	mipi_dsi_dcs_write(dsi, 0x57, (u8[]){ 0x23 }, 1);
	mipi_dsi_dcs_write(dsi, 0x58, (u8[]){ 0x45 }, 1);
	mipi_dsi_dcs_write(dsi, 0x59, (u8[]){ 0x67 }, 1);
	mipi_dsi_dcs_write(dsi, 0x5a, (u8[]){ 0x89 }, 1);
	mipi_dsi_dcs_write(dsi, 0x5b, (u8[]){ 0xab }, 1);
	mipi_dsi_dcs_write(dsi, 0x5c, (u8[]){ 0xcd }, 1);
	mipi_dsi_dcs_write(dsi, 0x5d, (u8[]){ 0xef }, 1);
	mipi_dsi_dcs_write(dsi, 0x5e, (u8[]){ 0x01 }, 1);
	mipi_dsi_dcs_write(dsi, 0x5f, (u8[]){ 0x14 }, 1);
	mipi_dsi_dcs_write(dsi, 0x60, (u8[]){ 0x15 }, 1);
	mipi_dsi_dcs_write(dsi, 0x61, (u8[]){ 0x0c }, 1);
	mipi_dsi_dcs_write(dsi, 0x62, (u8[]){ 0x0d }, 1);
	mipi_dsi_dcs_write(dsi, 0x63, (u8[]){ 0x0e }, 1);
	mipi_dsi_dcs_write(dsi, 0x64, (u8[]){ 0x0f }, 1);
	mipi_dsi_dcs_write(dsi, 0x65, (u8[]){ 0x10 }, 1);
	mipi_dsi_dcs_write(dsi, 0x66, (u8[]){ 0x11 }, 1);
	mipi_dsi_dcs_write(dsi, 0x67, (u8[]){ 0x08 }, 1);
	mipi_dsi_dcs_write(dsi, 0x68, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x69, (u8[]){ 0x0a }, 1);
	mipi_dsi_dcs_write(dsi, 0x6a, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x6b, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x6c, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x6d, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x6e, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x6f, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x70, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x71, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x72, (u8[]){ 0x06 }, 1);
	mipi_dsi_dcs_write(dsi, 0x73, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x74, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x75, (u8[]){ 0x14 }, 1);
	mipi_dsi_dcs_write(dsi, 0x76, (u8[]){ 0x15 }, 1);
	mipi_dsi_dcs_write(dsi, 0x77, (u8[]){ 0x11 }, 1);
	mipi_dsi_dcs_write(dsi, 0x78, (u8[]){ 0x10 }, 1);
	mipi_dsi_dcs_write(dsi, 0x79, (u8[]){ 0x0f }, 1);
	mipi_dsi_dcs_write(dsi, 0x7a, (u8[]){ 0x0e }, 1);
	mipi_dsi_dcs_write(dsi, 0x7b, (u8[]){ 0x0d }, 1);
	mipi_dsi_dcs_write(dsi, 0x7c, (u8[]){ 0x0c }, 1);
	mipi_dsi_dcs_write(dsi, 0x7d, (u8[]){ 0x06 }, 1);
	mipi_dsi_dcs_write(dsi, 0x7e, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x7f, (u8[]){ 0x0a }, 1);
	mipi_dsi_dcs_write(dsi, 0x80, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x81, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x82, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x83, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x84, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x85, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x86, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x87, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x88, (u8[]){ 0x08 }, 1);
	mipi_dsi_dcs_write(dsi, 0x89, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x8a, (u8[]){ 0x02 }, 1);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	gpd_set_page(gpd, 0x04);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	//VCORE
	mipi_dsi_dcs_write(dsi, 0x6c, (u8[]){ 0x15 }, 1);
	//Power control 2
	mipi_dsi_dcs_write(dsi, 0x6e, (u8[]){ 0x3b }, 1);
	//Power control 3
	mipi_dsi_dcs_write(dsi, 0x6f, (u8[]){ 0x53 }, 1);
	mipi_dsi_dcs_write(dsi, 0x3a, (u8[]){ 0xa4 }, 1);
	//Power control 4
	mipi_dsi_dcs_write(dsi, 0x8d, (u8[]){ 0x15 }, 1);
	mipi_dsi_dcs_write(dsi, 0x87, (u8[]){ 0xba }, 1);
	mipi_dsi_dcs_write(dsi, 0xb2, (u8[]){ 0xd1 }, 1);
	mipi_dsi_dcs_write(dsi, 0x26, (u8[]){ 0x76 }, 1);
	//vcom control 2
	mipi_dsi_dcs_write(dsi, 0x88, (u8[]){ 0x0b }, 1);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	gpd_set_page(gpd, 0x01);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	//Set panel operation mode and data complement setting
	mipi_dsi_dcs_write(dsi, 0x22, (u8[]){ 0x0a }, 1);
	//Display inversion
	mipi_dsi_dcs_write(dsi, 0x31, (u8[]){ 0x00 }, 1);
	//Vcom control 1
	mipi_dsi_dcs_write(dsi, 0x53, (u8[]){ 0x8a }, 1);
	//Vcom control 1
	mipi_dsi_dcs_write(dsi, 0x55, (u8[]){ 0x88 }, 1);
	//Power control 1
	mipi_dsi_dcs_write(dsi, 0x50, (u8[]){ 0xa6 }, 1);
	//Power control 1
	mipi_dsi_dcs_write(dsi, 0x51, (u8[]){ 0xa6 }, 1);
	//Source timing adjust
	mipi_dsi_dcs_write(dsi, 0x60, (u8[]){ 0x14 }, 1);
	//Positive gamma correction {
	mipi_dsi_dcs_write(dsi, 0xa0, (u8[]){ 0x08 }, 1);
	mipi_dsi_dcs_write(dsi, 0xa1, (u8[]){ 0x27 }, 1);
	mipi_dsi_dcs_write(dsi, 0xa2, (u8[]){ 0x36 }, 1);
	mipi_dsi_dcs_write(dsi, 0xa3, (u8[]){ 0x15 }, 1);
	mipi_dsi_dcs_write(dsi, 0xa4, (u8[]){ 0x17 }, 1);
	mipi_dsi_dcs_write(dsi, 0xa5, (u8[]){ 0x2b }, 1);
	mipi_dsi_dcs_write(dsi, 0xa6, (u8[]){ 0x1e }, 1);
	mipi_dsi_dcs_write(dsi, 0xa7, (u8[]){ 0x1f }, 1);
	mipi_dsi_dcs_write(dsi, 0xa8, (u8[]){ 0x96 }, 1);
	mipi_dsi_dcs_write(dsi, 0xa9, (u8[]){ 0x1c }, 1);
	mipi_dsi_dcs_write(dsi, 0xaa, (u8[]){ 0x28 }, 1);
	mipi_dsi_dcs_write(dsi, 0xab, (u8[]){ 0x7c }, 1);
	mipi_dsi_dcs_write(dsi, 0xac, (u8[]){ 0x1b }, 1);
	mipi_dsi_dcs_write(dsi, 0xad, (u8[]){ 0x1a }, 1);
	mipi_dsi_dcs_write(dsi, 0xae, (u8[]){ 0x4d }, 1);
	mipi_dsi_dcs_write(dsi, 0xaf, (u8[]){ 0x23 }, 1);
	mipi_dsi_dcs_write(dsi, 0xb0, (u8[]){ 0x29 }, 1);
	mipi_dsi_dcs_write(dsi, 0xb1, (u8[]){ 0x4b }, 1);
	mipi_dsi_dcs_write(dsi, 0xb2, (u8[]){ 0x5a }, 1);
	mipi_dsi_dcs_write(dsi, 0xb3, (u8[]){ 0x2c }, 1);
	//}
	//Negative gamma correction {
	mipi_dsi_dcs_write(dsi, 0xc0, (u8[]){ 0x08 }, 1);
	mipi_dsi_dcs_write(dsi, 0xc1, (u8[]){ 0x26 }, 1);
	mipi_dsi_dcs_write(dsi, 0xc2, (u8[]){ 0x36 }, 1);
	mipi_dsi_dcs_write(dsi, 0xc3, (u8[]){ 0x15 }, 1);
	mipi_dsi_dcs_write(dsi, 0xc4, (u8[]){ 0x17 }, 1);
	mipi_dsi_dcs_write(dsi, 0xc5, (u8[]){ 0x2b }, 1);
	mipi_dsi_dcs_write(dsi, 0xc6, (u8[]){ 0x1f }, 1);
	mipi_dsi_dcs_write(dsi, 0xc7, (u8[]){ 0x1f }, 1);
	mipi_dsi_dcs_write(dsi, 0xc8, (u8[]){ 0x96 }, 1);
	mipi_dsi_dcs_write(dsi, 0xc9, (u8[]){ 0x1c }, 1);
	mipi_dsi_dcs_write(dsi, 0xca, (u8[]){ 0x29 }, 1);
	mipi_dsi_dcs_write(dsi, 0xcb, (u8[]){ 0x7c }, 1);
	mipi_dsi_dcs_write(dsi, 0xcc, (u8[]){ 0x1a }, 1);
	mipi_dsi_dcs_write(dsi, 0xcd, (u8[]){ 0x19 }, 1);
	mipi_dsi_dcs_write(dsi, 0xce, (u8[]){ 0x4d }, 1);
	mipi_dsi_dcs_write(dsi, 0xcf, (u8[]){ 0x22 }, 1);
	mipi_dsi_dcs_write(dsi, 0xd0, (u8[]){ 0x29 }, 1);
	mipi_dsi_dcs_write(dsi, 0xd1, (u8[]){ 0x4b }, 1);
	mipi_dsi_dcs_write(dsi, 0xd2, (u8[]){ 0x59 }, 1);
	mipi_dsi_dcs_write(dsi, 0xd3, (u8[]){ 0x2c }, 1);
	//}
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	gpd_set_page(gpd, 0x00);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	//TE_ON
	mipi_dsi_dcs_write(dsi, 0x35, (u8[]){ 0x00 }, 1);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	mipi_dsi_dcs_exit_sleep_mode(dsi);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	msleep(140);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

static int gpd_panel_on(struct gpd_panel *gpd)
{
	struct mipi_dsi_device *dsi = gpd->dsi;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0)
		return ret;

	msleep(30);

	return 0;
}

static int gpd_panel_off(struct gpd_panel *gpd)
{
	struct mipi_dsi_device *dsi = gpd->dsi;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0)
		return ret;

	msleep(100);

	return 0;
}

static int gpd_panel_disable(struct drm_panel *panel)
{
	struct gpd_panel *gpd = to_gpd_panel(panel);

	if (!gpd->enabled)
		return 0;

	DRM_DEBUG("disable\n");

	if (gpd->backlight) {
		gpd->backlight->props.power = FB_BLANK_POWERDOWN;
		backlight_update_status(gpd->backlight);
	}

	gpd->enabled = false;

	return 0;
}

static int gpd_panel_unprepare(struct drm_panel *panel)
{
	struct gpd_panel *gpd = to_gpd_panel(panel);
	int ret;

	if (!gpd->prepared)
		return 0;

	DRM_DEBUG("unprepare\n");

	ret = gpd_panel_off(gpd);
	if (ret) {
		dev_err(panel->dev, "failed to set panel off: %d\n", ret);
		return ret;
	}

	regulator_disable(gpd->supply);
	if (gpd->reset_gpio)
		gpiod_set_value(gpd->reset_gpio, 0);

	gpd->prepared = false;

	return 0;
}

static int gpd_panel_prepare(struct drm_panel *panel)
{
	struct gpd_panel *gpd = to_gpd_panel(panel);
	int ret;
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	ret = regulator_enable(gpd->supply);
	if (ret < 0)
		return ret;

	msleep(50);

	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	if (gpd->prepared)
		return 0;
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	DRM_DEBUG("prepare\n");
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	if (gpd->reset_gpio) {
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
		gpiod_set_value(gpd->reset_gpio, 0);
		msleep(20);
	}
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	msleep(20);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	if (gpd->reset_gpio) {
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
		gpiod_set_value(gpd->reset_gpio, 1);
		msleep(20);
	}

	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	msleep(150);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	ret = gpd_panel_init(gpd);
	if (ret) {
		dev_err(panel->dev, "failed to init panel: %d\n", ret);
		goto poweroff;
	}
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	ret = gpd_panel_on(gpd);
	if (ret) {
		dev_err(panel->dev, "failed to set panel on: %d\n", ret);
		goto poweroff;
	}
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	gpd->prepared = true;

	return 0;

poweroff:
	regulator_disable(gpd->supply);
	if (gpd->reset_gpio)
		gpiod_set_value(gpd->reset_gpio, 0);
	return ret;
}

static int gpd_panel_enable(struct drm_panel *panel)
{
	struct gpd_panel *gpd = to_gpd_panel(panel);

	if (gpd->enabled)
		return 0;

	DRM_DEBUG("enable\n");

	if (gpd->backlight) {
		gpd->backlight->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(gpd->backlight);
	}

	gpd->enabled = true;

	return 0;
}

static const struct drm_display_mode default_mode = {
	.clock = 60000,
	.hdisplay = 720,
	.hsync_start = 720 + 10,
	.hsync_end = 720 + 10 + 4,
	.htotal = 720 + 10 + 4 + 13,
	.vdisplay = 1280,
	.vsync_start = 1280 + 48,
	.vsync_end = 1280 + 48 + 8,
	.vtotal = 1280 + 48 + 8 + 4,
	.vrefresh = 60,
};

static int gpd_panel_get_modes(struct drm_panel *panel)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(panel->drm, &default_mode);
	if (!mode) {
		dev_err(panel->drm->dev, "failed to add mode %ux%ux@%u\n",
				default_mode.hdisplay, default_mode.vdisplay,
				default_mode.vrefresh);
		return -ENOMEM;
	}

	drm_mode_set_name(mode);

	drm_mode_probed_add(panel->connector, mode);

	panel->connector->display_info.width_mm = 64;
	panel->connector->display_info.height_mm = 114;

	return 1;
}

static const struct drm_panel_funcs gpd_panel_funcs = {
		.disable = gpd_panel_disable,
		.unprepare = gpd_panel_unprepare,
		.prepare = gpd_panel_prepare,
		.enable = gpd_panel_enable,
		.get_modes = gpd_panel_get_modes,
};

static const struct of_device_id gpd_of_match[] = {
		{ .compatible = "gpd,novatek-1080p-vid", },
		{ }
};
MODULE_DEVICE_TABLE(of, gpd_of_match);

static int gpd_panel_add(struct gpd_panel *gpd)
{
	struct device *dev= &gpd->dsi->dev;
	struct device_node *np;
	int ret;

	gpd->mode = &default_mode;

	gpd->supply = devm_regulator_get(dev, "power");
	if (IS_ERR(gpd->supply))
		return PTR_ERR(gpd->supply);

	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	gpd->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	if (IS_ERR(gpd->reset_gpio)) {
		dev_err(dev, "cannot get reset-gpios %ld\n",
			PTR_ERR(gpd->reset_gpio));
		gpd->reset_gpio = NULL;
	} else {
		gpiod_direction_output(gpd->reset_gpio, 0);
	}
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	np = of_parse_phandle(dev->of_node, "backlight", 0);
	if (np) {
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
		gpd->backlight = of_find_backlight_by_node(np);
		of_node_put(np);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

		if (!gpd->backlight)
			return -EPROBE_DEFER;
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	}

	drm_panel_init(&gpd->base);
	gpd->base.funcs = &gpd_panel_funcs;
	gpd->base.dev = &gpd->dsi->dev;

	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	ret = drm_panel_add(&gpd->base);
	if (ret < 0)
		goto put_backlight;
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	return 0;

	put_backlight:
	if (gpd->backlight)
		put_device(&gpd->backlight->dev);

	return ret;
}

static void gpd_panel_del(struct gpd_panel *gpd)
{
	if (gpd->base.dev)
		drm_panel_remove(&gpd->base);

	if (gpd->backlight)
		put_device(&gpd->backlight->dev);
}

static int gpd_panel_probe(struct mipi_dsi_device *dsi)
{
	struct gpd_panel *gpd;
	int ret;

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO |
			MIPI_DSI_MODE_VIDEO_BURST |
			MIPI_DSI_MODE_VIDEO_SYNC_PULSE;
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	gpd = devm_kzalloc(&dsi->dev, sizeof(*gpd), GFP_KERNEL);
	if (!gpd) {
		return -ENOMEM;
	}
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	mipi_dsi_set_drvdata(dsi, gpd);
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	gpd->dsi = dsi;
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	ret = gpd_panel_add(gpd);
	if (ret < 0) {
		return ret;
	}
	printk(KERN_WARNING "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);

	return mipi_dsi_attach(dsi);
}

static int gpd_panel_remove(struct mipi_dsi_device *dsi)
{
	struct gpd_panel *gpd = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = gpd_panel_disable(&gpd->base);
	if (ret < 0)
		dev_err(&dsi->dev, "failed to disable panel: %d\n", ret);

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "failed to detach from DSI host: %d\n", ret);

	drm_panel_detach(&gpd->base);
	gpd_panel_del(gpd);

	return 0;
}

static void gpd_panel_shutdown(struct mipi_dsi_device *dsi)
{
	struct gpd_panel *gpd = mipi_dsi_get_drvdata(dsi);

	gpd_panel_disable(&gpd->base);
}

static struct mipi_dsi_driver gpd_panel_driver = {
	.driver = {
		.name = "panel-gpd-ilitek-720p",
		.of_match_table = gpd_of_match,
	},
	.probe = gpd_panel_probe,
	.remove = gpd_panel_remove,
	.shutdown = gpd_panel_shutdown,
};
module_mipi_dsi_driver(gpd_panel_driver);

MODULE_AUTHOR("Pierre-Hugues Husson <phh@phh.me>");
MODULE_DESCRIPTION("GPD XD ili9881c-based panel driver");
MODULE_LICENSE("GPL v2");
