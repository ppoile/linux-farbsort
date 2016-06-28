/*
 * Remote processor messaging - logger service
 *
 * Based on "Remote processor messaging - sample client driver" by
 *   Ohad Ben-Cohen <ohad@wizery.com>
 *   Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>

static void rpmsg_logger_cb(struct rpmsg_channel *rpdev, void *data, int len,
	void *priv, u32 src)
{
	dev_info(&rpdev->dev, "%s:%d: %s", rpdev->desc, src, (char *)data);
}

static int rpmsg_logger_probe(struct rpmsg_channel *rpdev)
{
	int ret;

	ret = rpmsg_send(rpdev, NULL, 0);
	if (ret) {
		dev_err(&rpdev->dev, "rpmsg_send failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static struct rpmsg_device_id rpmsg_logger_id_table[] = {
	{ .name	= "logger" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_logger_id_table);

static struct rpmsg_driver rpmsg_logger = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= rpmsg_logger_id_table,
	.probe		= rpmsg_logger_probe,
	.callback	= rpmsg_logger_cb,
};

static int __init rpmsg_logger_init(void)
{
	return register_rpmsg_driver(&rpmsg_logger);
}
module_init(rpmsg_logger_init);

static void __exit rpmsg_logger_fini(void)
{
	unregister_rpmsg_driver(&rpmsg_logger);
}
module_exit(rpmsg_logger_fini);

MODULE_DESCRIPTION("Remote processor logger driver");
MODULE_LICENSE("GPL v2");
