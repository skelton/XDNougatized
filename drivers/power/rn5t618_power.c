/*
 * Battery driver for rn5t618 PMIC
 *
 * Copyright 2016 Pierre-Hugues Husson <phh@phh.me>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/mfd/rn5t618.h>

#define RN5T618_ADCCNT3_ADRQ			0x30
#define RN5T618_ADCCNT3_ADRQ_AUTO		0x20

#define RN5T618_CHGSTATE_STATE_MSK		0x1f
#define RN5T618_CHGSTATE_CHG_OFF 		0
#define RN5T618_CHGSTATE_CHG_READY_VADP		1
#define RN5T618_CHGSTATE_CHG_TRICKLE		2
#define RN5T618_CHGSTATE_CHG_RAPID		3
#define RN5T618_CHGSTATE_CHG_COMPLETE		4
#define RN5T618_CHGSTATE_SUSPEND		5
#define RN5T618_CHGSTATE_VCHG_OVER_VOL		6
#define RN5T618_CHGSTATE_BAT_ERROR		7
#define RN5T618_CHGSTATE_NO_BAT			8
#define RN5T618_CHGSTATE_BAT_OVER_VOL		9
#define RN5T618_CHGSTATE_BAT_TEMP_ERR		10
#define RN5T618_CHGSTATE_DIE_ERR		11
#define RN5T618_CHGSTATE_DIE_SHUTDOWN		12
#define RN5T618_CHGSTATE_NO_BAT2		13
#define RN5T618_CHGSTATE_CHG_READY_VUSB		14
#define RN5T618_CHGSTATE_SRC_MSK		0xc0
#define RN5T618_CHGSTATE_SRC_ADP		0x40
#define RN5T618_CHGSTATE_SRC_USB		0x80

static int rn5t618_read_adc(struct rn5t618 *rn5t618, int idx)
{
	int ret, value;
	u8 buf[2];

	/* This ADC has 8 inputs */
	if(idx < 0 || idx >= 8)
		return -1;
	/* Returns there is no battery if communication fails */
	ret = regmap_bulk_read(rn5t618->regmap, RN5T618_ILIMDATAH + 2*idx, buf, 2);
	if (ret < 0)
		return ret;
	value = buf[0] << 4 | (buf[1] & 0xf);

	/* This is a 12-bits ADC, with a 2.5V reference */
	return value * 2500 / 4096;
}

static int rn5t618_batt_status(struct rn5t618 *rn5t618)
{
	unsigned int status;
	int ret, charge_state, supply_state;

	ret = regmap_read(rn5t618->regmap, RN5T618_CHGSTATE, &status);
	if (ret < 0)
		return ret;

	charge_state = status&RN5T618_CHGSTATE_STATE_MSK;
	supply_state = status&RN5T618_CHGSTATE_SRC_MSK;

	/* No external supply */
	if(supply_state == 0)
		return POWER_SUPPLY_STATUS_DISCHARGING;

	switch (charge_state) {
	case RN5T618_CHGSTATE_CHG_COMPLETE:
		return POWER_SUPPLY_STATUS_FULL;
	case RN5T618_CHGSTATE_CHG_RAPID:
	case RN5T618_CHGSTATE_CHG_TRICKLE: //Should this be FULL?
		return POWER_SUPPLY_STATUS_CHARGING;
	case RN5T618_CHGSTATE_CHG_READY_VADP:
	case RN5T618_CHGSTATE_BAT_ERROR:
	case RN5T618_CHGSTATE_NO_BAT:
	case RN5T618_CHGSTATE_BAT_OVER_VOL:
	case RN5T618_CHGSTATE_BAT_TEMP_ERR:
	case RN5T618_CHGSTATE_DIE_ERR:
	case RN5T618_CHGSTATE_NO_BAT2:
	case RN5T618_CHGSTATE_CHG_READY_VUSB:
		return POWER_SUPPLY_STATUS_NOT_CHARGING;
	case RN5T618_CHGSTATE_CHG_OFF:
	case RN5T618_CHGSTATE_SUSPEND:
	case RN5T618_CHGSTATE_VCHG_OVER_VOL:
	case RN5T618_CHGSTATE_DIE_SHUTDOWN:
		return POWER_SUPPLY_STATUS_DISCHARGING;

	default:
		return POWER_SUPPLY_STATUS_UNKNOWN;
	}
}

static int rn5t618_supply(struct rn5t618 *rn5t618, int usb)
{
	unsigned int status;
	int ret, supply_state;

	/* Returns there is no supply if communication fails */
	ret = regmap_read(rn5t618->regmap, RN5T618_CHGSTATE, &status);
	if (ret < 0)
		return 0;

	supply_state = status&RN5T618_CHGSTATE_SRC_MSK;
	if(usb)
		return !!(supply_state & RN5T618_CHGSTATE_SRC_USB);
	return !!(supply_state & RN5T618_CHGSTATE_SRC_ADP);
}

static int rn5t618_batt_online(struct rn5t618 *rn5t618)
{
	unsigned int status;
	int ret, charge_state;

	/* Returns there is no battery if communication fails */
	ret = regmap_read(rn5t618->regmap, RN5T618_CHGSTATE, &status);
	if (ret < 0)
		return 0;

	charge_state = status&RN5T618_CHGSTATE_STATE_MSK;

	switch (charge_state) {
	case RN5T618_CHGSTATE_CHG_COMPLETE:
	case RN5T618_CHGSTATE_CHG_RAPID:
	case RN5T618_CHGSTATE_CHG_TRICKLE:
	case RN5T618_CHGSTATE_CHG_OFF:
	case RN5T618_CHGSTATE_SUSPEND:
	case RN5T618_CHGSTATE_VCHG_OVER_VOL:
	case RN5T618_CHGSTATE_DIE_SHUTDOWN:
		return 1;
	}
	return 0;
}

static int rn5t618_ac_get_prop(struct power_supply *psy,
			      enum power_supply_property psp,
			      union power_supply_propval *val)
{
	struct rn5t618 *rn5t618 = dev_get_drvdata(psy->dev.parent);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = rn5t618_supply(rn5t618, 0);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* VADP divided by 3 before going into ADC */
		val->intval = rn5t618_read_adc(rn5t618, 2)*3;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static enum power_supply_property rn5t618_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static int rn5t618_usb_get_prop(struct power_supply *psy,
			       enum power_supply_property psp,
			       union power_supply_propval *val)
{
	struct rn5t618 *rn5t618 = dev_get_drvdata(psy->dev.parent);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = rn5t618_supply(rn5t618, 1);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* VUSB divided by 3 before going into ADC */
		val->intval = rn5t618_read_adc(rn5t618, 3)*3;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static enum power_supply_property rn5t618_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static int rn5t618_bat_get_property(struct power_supply *psy,
				   enum power_supply_property psp,
				   union power_supply_propval *val)
{
	struct rn5t618 *rn5t618 = dev_get_drvdata(psy->dev.parent);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = rn5t618_batt_status(rn5t618);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = rn5t618_batt_online(rn5t618);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* VBAT is halfed before going into ADC */
		val->intval = rn5t618_read_adc(rn5t618, 1)*2;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static enum power_supply_property rn5t618_bat_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static const struct power_supply_desc rn5t618_ac_desc = {
	.name		= "rn5t618-ac",
	.type		= POWER_SUPPLY_TYPE_MAINS,
	.properties	= rn5t618_ac_props,
	.num_properties	= ARRAY_SIZE(rn5t618_ac_props),
	.get_property	= rn5t618_ac_get_prop,
};

static const struct power_supply_desc rn5t618_battery_desc = {
	.name		= "rn5t618-battery",
	.type		= POWER_SUPPLY_TYPE_BATTERY,
	.properties	= rn5t618_bat_props,
	.num_properties	= ARRAY_SIZE(rn5t618_bat_props),
	.get_property	= rn5t618_bat_get_property,
};

static const struct power_supply_desc rn5t618_usb_desc = {
	.name		= "rn5t618-usb",
	.type		= POWER_SUPPLY_TYPE_USB,
	.properties	= rn5t618_usb_props,
	.num_properties	= ARRAY_SIZE(rn5t618_usb_props),
	.get_property	= rn5t618_usb_get_prop,
};

static int rn5t618_setup_adc(struct rn5t618 *rn5t618) {
	int ret;

	/* Disable ADC to allow writing registers */
	ret = regmap_write_bits(rn5t618->regmap, RN5T618_ADCCNT3,
			RN5T618_ADCCNT3_ADRQ,
			0);
	if (ret < 0)
		goto err;

	/* Scan all ADC inputs */
	ret = regmap_write_bits(rn5t618->regmap, RN5T618_ADCCNT1,
			0xff,
			0xff);
	if (ret < 0)
		goto err;

	/* Scan ADC inputs every 250ms */
	ret = regmap_write_bits(rn5t618->regmap, RN5T618_ADCCNT2,
			0xff,
			0x0);
	if (ret < 0)
		goto err;

	/* Disable ADC to allow writing registers */
	ret = regmap_write_bits(rn5t618->regmap, RN5T618_ADCCNT3,
			RN5T618_ADCCNT3_ADRQ,
			RN5T618_ADCCNT3_ADRQ_AUTO);
	if (ret < 0)
		goto err;
err:
	return ret;

}

static int rn5t618_power_probe(struct platform_device *pdev)
{
	struct rn5t618 *rn5t618 = dev_get_drvdata(pdev->dev.parent);
	struct power_supply *supply;
	int ret;

	dev_set_drvdata(&pdev->dev, rn5t618);

	supply = devm_power_supply_register(&pdev->dev,
			&rn5t618_ac_desc, NULL);
	if (IS_ERR(supply))
		return PTR_ERR(supply);

	supply = devm_power_supply_register(&pdev->dev,
			&rn5t618_battery_desc, NULL);
	if (IS_ERR(supply))
		return PTR_ERR(supply);


	supply = devm_power_supply_register(&pdev->dev,
			&rn5t618_usb_desc, NULL);
	if (IS_ERR(supply))
		return PTR_ERR(supply);

	ret = rn5t618_setup_adc(rn5t618);
	dev_err(&pdev->dev, "Setting up ADCs failed with err:%d\n", ret);

	return 0;

}

static struct platform_driver rn5t618_power_driver = {
	.probe = rn5t618_power_probe,
	.driver = {
		.name = "rn5t618-power",
	},
};

module_platform_driver(rn5t618_power_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Power supply driver for RN5T618");
MODULE_ALIAS("platform:rn5t618-power");
