/*
* Copyright (C) 2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*/

#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/file.h>
#include <linux/uaccess.h>

#define FL3236_SHUTDOWN_REG     0x00
#define FL3236_PWM_REG          0x01
#define FL3236_UPDATE_REG       0x25
#define FL3236_LED_REG          0x26
#define FL3236_CONTROL_REG      0x4a
#define FL3236_FREQUENCY_REG    0x4b
#define FL3236_RESET_REG        0x4f

#define SHUTDOWN_REG_NORMAL      0x01
#define SHUTDOWN_REG_SHUTDOWN    0x00
#define UPDATE_REG_UPDATE        0x00
#define CONTROL_REG_SHUTDOWN_ALL 0x01
#define CONTROL_REG_WORK_ALL     0x00
#define FREQUENCY_REG_3KHZ       0x00
#define FREQUENCY_REG_22KHZ      0x01
#define RESET_REG_RESET          0x00

#define REG_GROUP_COUNT       12
#define REG_COUNT             36

#define CMD_SINGLE            (101)
#define CMD_GROUP_MASK        (102)
#define CMD_GROUP_LINEAR      (103)
#define CMD_GROUP_BATCH       (104)
#define CMD_GROUP_ALL         (105)

struct fl3236_dev {
	struct cdev dev;
	struct i2c_client *client;
	struct mutex lock;
};

struct fl3236_dev *fl3236_devp;
dev_t devno = 0;

static ssize_t fl3236_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    //printk("fl3236 write \n");
	return 0;
}

static ssize_t fl3236_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    //printk("fl3236 read \n");
	return 0;
}

static int fl3236_set_data(u8 base_reg, int template, u8 count, u8 data)
{
    u8 i;

	if (template == 0)
	    return -1;
	
	for (i=0;i<count;i++)
	{
	    if (0x01 & template)
		{
		    i2c_smbus_write_byte_data(fl3236_devp->client, base_reg+i, data); 
		    template = template >> 1;
		}
		else
		{
		    template = template >> 1;
		    continue;
		}
	}
	return 0;
}

static int fl3236_set_data_all(u8 base_reg, u8 count, u8 data)
{
    u8 i;
	for (i=0;i<count;i++)
	{
	    i2c_smbus_write_byte_data(fl3236_devp->client, base_reg+i, data);
	}
	return 0;
}

static long fl3236_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct fl3236_dev *dev = filp->private_data;
    u8 user_reg, user_data, user_count;
	int user_template;
	user_reg = 0xFF & (arg >> 8);
	user_data = 0xFF & arg;

	//printk("%s enter kernel space %x, %lx\n", __func__, cmd, arg);

	mutex_lock(&dev->lock);
	switch(cmd)
	{
	    case CMD_SINGLE:
		    i2c_smbus_write_byte_data(fl3236_devp->client,user_reg, user_data);
			i2c_smbus_write_byte_data(fl3236_devp->client,FL3236_UPDATE_REG, UPDATE_REG_UPDATE);
		    break;

        case CMD_GROUP_MASK:
		    user_template = 0x0FFF & (arg >> 16);
		    fl3236_set_data(user_reg,user_template,REG_GROUP_COUNT,user_data);
			i2c_smbus_write_byte_data(fl3236_devp->client,FL3236_UPDATE_REG, UPDATE_REG_UPDATE);
		    break;

		case CMD_GROUP_LINEAR:
		    user_reg = 0xFF & (arg >> 24);
			user_data = 0xFF & (arg >> 16);
			i2c_smbus_write_byte_data(fl3236_devp->client,user_reg, user_data);
			user_data = 0xFF & (arg >> 8);
			i2c_smbus_write_byte_data(fl3236_devp->client,user_reg+1, user_data);
			user_data = 0xFF & (arg);
			i2c_smbus_write_byte_data(fl3236_devp->client,user_reg+2, user_data);
			
			i2c_smbus_write_byte_data(fl3236_devp->client,FL3236_UPDATE_REG, UPDATE_REG_UPDATE);
		    break;

        case CMD_GROUP_BATCH:
		    user_count = 0xFF & (arg >> 16); 
			fl3236_set_data_all(user_reg,user_count,user_data);
			i2c_smbus_write_byte_data(fl3236_devp->client,FL3236_UPDATE_REG, UPDATE_REG_UPDATE);
		    break;

        case CMD_GROUP_ALL:
		    if (user_reg == FL3236_LED_REG || user_reg == FL3236_PWM_REG)
			{
		        fl3236_set_data_all(user_reg,REG_COUNT,user_data); 
				i2c_smbus_write_byte_data(fl3236_devp->client,FL3236_UPDATE_REG, UPDATE_REG_UPDATE);
			}
		    break;

		default:
	    printk("fl3236 ioctrl: command not support \n");
		break;
	}
    mutex_unlock(&dev->lock);
	return 0;
}

static int fl3236_open(struct inode *inode, struct file *filp)
{
    filp->private_data = fl3236_devp;

	return 0;
}

static int fl3236_release(struct inode *inode, struct file *filp)
{
    filp->private_data = NULL;

	return 0;
}

static const struct file_operations fl3236_fops = {
    .owner = THIS_MODULE,
	.open = fl3236_open,
	.release = fl3236_release,
	.read = fl3236_read,
	.write = fl3236_write,
	.unlocked_ioctl = fl3236_ioctl,
};

static struct miscdevice fl3236_device = {
    .minor = MISC_DYNAMIC_MINOR,
	.name = "fl3236",
	.fops = &fl3236_fops,
};


static int fl3236_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	int err;

    //dev init
    devno = MKDEV(0,0);
	err = alloc_chrdev_region(&devno, 0, 1, "fl3236");
	if (err < 0)
	{
	    goto err_alloc;
	}

    fl3236_devp = kmalloc(sizeof(struct fl3236_dev), GFP_KERNEL);
	if (!fl3236_devp)
	{
	  err = -ENOMEM;
	  goto err_alloc;
	}
    memset(fl3236_devp, 0, sizeof(struct fl3236_dev));

	cdev_init(&fl3236_devp->dev, &fl3236_fops);
	fl3236_devp->dev.owner = THIS_MODULE;
	fl3236_devp->dev.ops = &fl3236_fops;
	mutex_init(&fl3236_devp->lock);
	fl3236_devp->client = client;
	i2c_set_clientdata(client, fl3236_devp);

	err = cdev_add(&fl3236_devp->dev, devno, 1);
	if (err) {
	goto err_add;
	}

	err = misc_register(&fl3236_device);
	if (err) {
	 goto err_regdevice;
	}
    
	//register init
    fl3236_set_data_all(FL3236_PWM_REG, REG_COUNT, 0xaf); 
    i2c_smbus_write_byte_data(fl3236_devp->client, FL3236_SHUTDOWN_REG, SHUTDOWN_REG_NORMAL);
	i2c_smbus_write_byte_data(fl3236_devp->client, FL3236_FREQUENCY_REG, FREQUENCY_REG_22KHZ);
	i2c_smbus_write_byte_data(fl3236_devp->client, FL3236_UPDATE_REG, UPDATE_REG_UPDATE);

	return 0;
err_regdevice:
    misc_deregister(&fl3236_device);
err_add:
    cdev_del(&fl3236_devp->dev);
err_alloc:
    unregister_chrdev_region(devno, 0);

	return err;

}

static int fl3236_remove(struct i2c_client *client)
{
    misc_deregister(&fl3236_device);
	cdev_del(&fl3236_devp->dev);
	kfree(fl3236_devp);
	unregister_chrdev_region(devno, 0);
	i2c_unregister_device(client);

	printk("[fl3236] remove module \n");
	return 0;
}

static const struct i2c_device_id fl3236_id[] = {
    { "fl3236",  0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, fl3236_id);

static const struct of_device_id of_fl3236_leds_match[] = {
    { .compatible = "issi,fl3236", },
	{},
};

MODULE_DEVICE_TABLE(of, of_fl3236_leds_match);

static struct i2c_driver fl3236_driver = {
    .driver = {
		.name   = "fl3236x",
		.of_match_table = of_match_ptr(of_fl3236_leds_match),
	},
	.probe      = fl3236_probe,
	.remove     = fl3236_remove,
	.id_table   = fl3236_id,
};

module_i2c_driver(fl3236_driver);
