/*
 * Regulator driver for Ricoh RN5T618 PMIC
 *
 * Copyright (C) 2016 Pierre-Hugues Husson <phh@phh.me>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/mfd/rn5t618.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/platform_device.h>

#define NUM_TIME_REGS	(RN5T618_RTC_YEAR - RN5T618_RTC_SEC + 1)
static int rc5t619_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct rn5t618 *rn5t618 = dev_get_drvdata(dev->parent);
	u8 rtc_data[NUM_TIME_REGS];
	int ret;
	int century = 0;

	ret = regmap_bulk_read(rn5t618->regmap, RN5T618_RTC_SEC, rtc_data,
		NUM_TIME_REGS);
	if (ret < 0) {
		dev_err(dev, "RTC read time failed with err:%d\n", ret);
		return ret;
	}

	if(rtc_data[5]&0x80)
		century = 1;

	tm->tm_sec = bcd2bin(rtc_data[0]);
	tm->tm_min = bcd2bin(rtc_data[1]);
	tm->tm_hour = bcd2bin(rtc_data[2]);
	tm->tm_wday = bcd2bin(rtc_data[3]);
	tm->tm_mday = bcd2bin(rtc_data[4]);
	tm->tm_mon = bcd2bin(rtc_data[5] & 0x1f) - 1;
	tm->tm_year = bcd2bin(rtc_data[6]) + 100 * century;

	return ret;
}

static int rc5t619_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct rn5t618 *rn5t618 = dev_get_drvdata(dev->parent);
	unsigned char rtc_data[NUM_TIME_REGS];
	int ret;

	rtc_data[0] = bin2bcd(tm->tm_sec);
	rtc_data[1] = bin2bcd(tm->tm_min);
	rtc_data[2] = bin2bcd(tm->tm_hour);
	rtc_data[3] = bin2bcd(tm->tm_wday);
	rtc_data[4] = bin2bcd(tm->tm_mday);
	rtc_data[5] = bin2bcd(tm->tm_mon + 1);
	rtc_data[6] = bin2bcd(tm->tm_year - 100);

	ret = regmap_bulk_write(rn5t618->regmap, RN5T618_RTC_SEC, rtc_data,
		NUM_TIME_REGS);
	if (ret < 0) {
		dev_err(dev, "RTC set time failed with error %d\n", ret);
		return ret;
	}

	return ret;
}

static const struct rtc_class_ops rc5t619_rtc_ops = {
	.read_time	= rc5t619_rtc_read_time,
	.set_time	= rc5t619_rtc_set_time,
};

static int rn5t618_rtc_probe(struct platform_device *pdev)
{
	struct rn5t618 *rn5t618 = dev_get_drvdata(pdev->dev.parent);
	struct rtc_device	*rtc;
	int ret;

	if(rn5t618->variant != RC5T619)
		return -ENODEV;

	rtc = devm_rtc_device_register(&pdev->dev, pdev->name,
		&rc5t619_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		ret = PTR_ERR(rtc);
		dev_err(&pdev->dev, "RTC device register: err %d\n", ret);
		return ret;
	}

	ret = regmap_write_bits(rn5t618->regmap, RN5T618_RTC_CTRL1,
			RN5T618_RTC_CTRL1_24HOURS,
			RN5T618_RTC_CTRL1_24HOURS);
	if (ret < 0) {
		dev_err(&pdev->dev, "RTC set 24-hours mode failed with err:%d\n", ret);
		return ret;
	}

	return 0;
}

static struct platform_driver rn5t618_rtc_driver = {
	.probe = rn5t618_rtc_probe,
	.driver = {
		.name	= "rn5t618-rtc",
	},
};

module_platform_driver(rn5t618_rtc_driver);

MODULE_AUTHOR("Pierre-Hugues Husson <phh@phh.me>");
MODULE_DESCRIPTION("RC5T619 RTC driver");
MODULE_LICENSE("GPL v2");
