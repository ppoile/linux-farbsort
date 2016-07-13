/*
 * Driver to allocate ADC resources and enable power and clock for the ADC to
 * use the ADC from the PRU.
 *
 * bz bbv / Urs Fässler; orignal by Rachna Patil:
 *
 * TI Touch Screen / ADC MFD driver
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/of_device.h>

static	int ti_tscadc_probe(struct platform_device *pdev)
{
	struct resource		*res;
	struct clk		*clk;
	int			err;
	void __iomem		*tscadc_base;

	err = platform_get_irq(pdev, 0);
	if (err < 0) {
		dev_err(&pdev->dev, "no irq ID is specified.\n");
		goto ret;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	tscadc_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(tscadc_base))
		return PTR_ERR(tscadc_base);

	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	/*
	 * The TSC_ADC_Subsystem has 2 clock domains
	 * OCP_CLK and ADC_CLK.
	 * The ADC clock is expected to run at target of 3MHz,
	 * and expected to capture 12-bit data at a rate of 200 KSPS.
	 * The TSC_ADC_SS controller design assumes the OCP clock is
	 * at least 6x faster than the ADC clock.
	 */
	clk = clk_get(&pdev->dev, "adc_tsc_fck");
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed to get TSC fck\n");
		err = PTR_ERR(clk);
		goto err_disable_clk;
	}
	clk_put(clk);

	device_init_wakeup(&pdev->dev, true);
	return 0;

err_disable_clk:
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
ret:
	return err;
}

static int ti_tscadc_remove(struct platform_device *pdev)
{
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static const struct of_device_id ti_tscadc_dt_ids[] = {
	{ .compatible = "ti,am3359-tscadc", },
	{ }
};
MODULE_DEVICE_TABLE(of, ti_tscadc_dt_ids);

static struct platform_driver ti_tscadc_driver = {
	.driver = {
		.name   = "ti_am3359-tscadc",
		.of_match_table = ti_tscadc_dt_ids,
	},
	.probe	= ti_tscadc_probe,
	.remove	= ti_tscadc_remove,

};

module_platform_driver(ti_tscadc_driver);

MODULE_DESCRIPTION("ADC set up for usage from the PRU");
MODULE_AUTHOR("Urs Fässler <urs.fassler@bbv.ch>");
MODULE_LICENSE("GPL");
