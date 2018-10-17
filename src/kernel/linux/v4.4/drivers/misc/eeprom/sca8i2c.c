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
/*
 * sca8i2c.c - handle Sensory EEPROMs
 */
 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/acpi.h>
#include <linux/i2c.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/file.h>
#include <linux/uaccess.h>

struct sca8i2c_dev {
    struct cdev dev;
    struct i2c_client *client;
    /*
    u8 *r_buf;
	  u8 *wr_buf;
	  */
	  int flag;
		struct mutex lock;
};

struct sca8i2c_dev *sca8i2c_devp;
dev_t devno = 0;

/* Size of EEPROM in bytes */
#define BUFF_COUNT 9

#define SCA8I2C_WRITE_CHALLENGE  1
#define SCA8I2C_READ_RESPONSE  2
#define SCA8I2C_READ_STATUS  3
#define SCA8I2C_READ_VERSION  4

static const struct of_device_id sca8i2c_of_match[] = {
	{ .compatible = "sensory,sca8i2c"},
	{ }
};
MODULE_DEVICE_TABLE(of, sca8i2c_of_match);

static const struct i2c_device_id sca8i2c_i2c_id[] = {
	{ "sca8i2c", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sca8i2c_i2c_id);

static const struct acpi_device_id sca8i2c_acpi_ids[] = {
	{ "sca8i2c", 0 },
	{ }
};

MODULE_DEVICE_TABLE(acpi, sca8i2c_acpi_ids);

/****************************************************************************
 * add APIs for I2C Read/Write
 ***************************************************************************/

ssize_t sca8i2c_eeprom_read(struct i2c_client *client, u8 *buf, u8 reg_addr, int rd_len)
{
	  int ret = 0;
	  int i = 0;
    
    struct i2c_msg msg[2];
    u8 *w_buf = NULL;
    u8 *r_buf = NULL;
  
    memset(msg, 0, 2 * sizeof(struct i2c_msg));
    
    w_buf = kzalloc(1, GFP_KERNEL);
    if (w_buf == NULL)
        return -1;
    
    r_buf = kzalloc(rd_len, GFP_KERNEL);
    if (r_buf == NULL)
        return -1; 
  
    *w_buf = reg_addr;
    msg[0].addr = client->addr;
    msg[0].flags = 0;  //write
    msg[0].len = 1;
    msg[0].buf = w_buf;  //register addr 
 
    msg[1].addr = client->addr;
    msg[1].flags = 1;  //read
    msg[1].len = rd_len;
    msg[1].buf = r_buf;  

    ret = i2c_transfer(client->adapter, msg, 2); 
    
    memcpy(buf, r_buf, rd_len); 
    printk("%s Current data is %d:\n", __func__, ret);
    
    for(i = 0; i < rd_len; i++ )
    {
    	printk("-- buf[%d] = %02x", i, *(buf + i));
    }
    printk("\n");   
 
    kfree(w_buf);
    kfree(r_buf);
    return ret;
}

ssize_t sca8i2c_eeprom_write(struct i2c_client *client, const u8 *wr_buf, int wr_len)
{	
    int ret = 0;
    struct i2c_msg msg;
    u8 *w_buf = NULL;
  
    memset(&msg, 0, sizeof(struct i2c_msg));
  
    w_buf = kzalloc(wr_len, GFP_KERNEL);
    if (w_buf == NULL)
        return -1;
    
    memcpy(w_buf, wr_buf, wr_len);
 
    msg.addr = client->addr;
    msg.flags = 0;  //write
    msg.len = wr_len;
    msg.buf = w_buf; 
 
    ret = i2c_transfer(client->adapter, &msg, 1); 
     
    kfree(w_buf);
    return ret;
}

static ssize_t sca8i2c_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct sca8i2c_dev *dev = filp->private_data;
	int err = 0, i = 0;
	u8 *wr_buf = NULL;
	
	printk("Enter %s: count = %zx, pos = %lld\n", __func__, count, *f_pos);
	
	wr_buf = kzalloc(BUFF_COUNT, GFP_KERNEL);
	if (wr_buf == NULL)
		return -1;
	
	mutex_lock(&dev->lock);
	if (copy_from_user(wr_buf, buf, count)){
      	goto Err_Case;
	}
	printk("%s before try to write data\n", __func__);
	for (i = 0; i < count; i++)
	{
		printk("Write data: buf[%d] = %2x\n", i, *(wr_buf+i));
	}
		
	switch (dev->flag)
		{
			case SCA8I2C_WRITE_CHALLENGE:
				err = sca8i2c_eeprom_write(dev->client, wr_buf, count);
				if (err == 1)
				{
				   printk("%s try to write data success\n",__func__);
				   err = count;
				}
				else{
					printk("%s try to write data fail:%d\n",__func__, err);
				}
				break;
      default:
      	goto Err_Case;
		}
	
	
	mutex_unlock(&dev->lock);
	kfree(wr_buf);
	
	return err;
	
Err_Case:
	mutex_unlock(&dev->lock); 	
	kfree(wr_buf);
  return -EFAULT;
}

static ssize_t sca8i2c_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct sca8i2c_dev *dev = filp->private_data;
	int err = 0;
	u8 *r_buf = NULL;
	u8 reg_addr = 0x0;
	printk("Enter %s : flag = %d, count = %zx, pos = %lld \n", __func__, dev->flag, count, *f_pos);
	printk("%s before try to read data: %d\n", __func__, reg_addr);
		
	r_buf = kzalloc(BUFF_COUNT, GFP_KERNEL);
	if (r_buf == NULL){
			return -EFAULT;
	}
	memset(r_buf, 0, BUFF_COUNT);
	
	mutex_lock(&dev->lock);
	//set register address
	switch (dev->flag)
		{
      case SCA8I2C_READ_RESPONSE:
      	reg_addr = 0x66;
      	break;
      case SCA8I2C_READ_STATUS:
      	reg_addr = 0x10;
      	break;
      case SCA8I2C_READ_VERSION:
      	reg_addr = 0x22;
      	break;
      default:
      	goto Err_Case;;
		}
		
	  //read data according to register address and count
		err = sca8i2c_eeprom_read(dev->client, r_buf, reg_addr, count);
		if (err == 2)
		{
		   printk("%s t try to read data success\n",__func__);
		   err = count;
		}
		else{
			goto Err_Case;
		}
	
	  //copy to user space
		if (copy_to_user(buf, r_buf, count))
			goto Err_Case;
			
	mutex_unlock(&dev->lock);		
	kfree(r_buf);
	return err;;
Err_Case:
			mutex_unlock(&dev->lock);
			kfree(r_buf);
			return -EFAULT;	
}
static long sca8i2c_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
		struct sca8i2c_dev *dev = filp->private_data;
		printk("%s enter kernel space %u, %lu\n", __func__, cmd, arg);
		
		mutex_lock(&dev->lock);
		switch (arg)
		{			
			case SCA8I2C_WRITE_CHALLENGE:
			case SCA8I2C_READ_RESPONSE:
      case SCA8I2C_READ_STATUS:
      case SCA8I2C_READ_VERSION:
				dev->flag = (int)arg;
      	printk("%s Enter setting segment:flag = %d!\n", __func__, dev->flag);
				break;
				
      default:
				mutex_unlock(&dev->lock);
      	return -EINVAL;
		}
		mutex_unlock(&dev->lock);
		
		return 0;
}

static int sca8i2c_open(struct inode *inode, struct file *filp)
{
	filp->private_data = sca8i2c_devp;
	
	return 0;
}

static int sca8i2c_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static const struct file_operations	sca8i2c_fops = {
						   .owner		= THIS_MODULE,
						   .open = sca8i2c_open,
						   .release = sca8i2c_release,
	             .read = sca8i2c_read,
	             .write = sca8i2c_write,
						   .unlocked_ioctl	= sca8i2c_ioctl,
						   };
			   
static struct miscdevice sca8i2c_device = {
						.minor = MISC_DYNAMIC_MINOR,
						.name  = "sca8i2c",
						.fops  = &sca8i2c_fops,
						};

/*-------------------------------------------------------------------------*/

static int sca8i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	/*
	u8 *r_buf = NULL;
	u8 *wr_buf = NULL;
  */
  devno = MKDEV(0,0);  
  err = alloc_chrdev_region(&devno, 0, 1, "SCA8I2C");
  if(err < 0){
  	goto err_alloc;
  }
  
  sca8i2c_devp = kmalloc(sizeof(struct sca8i2c_dev), GFP_KERNEL);
  if (!sca8i2c_devp){
		err = -ENOMEM;
		goto err_alloc;
  }
  memset(sca8i2c_devp, 0, sizeof(struct sca8i2c_dev));
  
  cdev_init(&sca8i2c_devp->dev, &sca8i2c_fops);
  sca8i2c_devp->dev.owner = THIS_MODULE;
  sca8i2c_devp->dev.ops = &sca8i2c_fops;
	mutex_init(&sca8i2c_devp->lock);
	sca8i2c_devp->client = client;
  i2c_set_clientdata(client, sca8i2c_devp);
  
  err = cdev_add(&sca8i2c_devp->dev, devno, 1);
  if (err) {
		goto err_add;
	}

	err = misc_register(&sca8i2c_device);
	if (err) {
		goto err_regdevice;
	}
	/*
	wr_buf = kzalloc(BUFF_COUNT, GFP_KERNEL);
	if (wr_buf == NULL)
	{
		err = -1; 
		goto err_regdevice;		
	}
	
	*wr_buf = 0x55;
	*(wr_buf + 1) = 0x11;
	*(wr_buf + 2) = 0x22;
	*(wr_buf + 3) = 0x33;	
	*(wr_buf + 4) = 0x44;
	*(wr_buf + 5) = 0x55;
	*(wr_buf + 6) = 0x66;
	*(wr_buf + 7) = 0x77;
	*(wr_buf + 8) = 0x88;
	
	err = sca8i2c_eeprom_write(client, wr_buf, 9);
	if (err == 1)
	{
	   printk("%s try to write data success\n",__func__);
	}
	else{
		printk("%s try to write data fail:%d\n",__func__, err);
	}
	
	r_buf = kzalloc(BUFF_COUNT, GFP_KERNEL);
	if (r_buf == NULL)
	{
		err = -1; 		
  	kfree(wr_buf);
		goto err_regdevice;		
	}
	
	err = sca8i2c_eeprom_read(client, r_buf, 0x66, 8);
	if (err == 2)
	{
		printk("%s try to read data success!!!\n", __func__);
	}
	else
	{
		printk("%s try to read data error!!!\n", __func__);
		err = -1; 
	}
	
	kfree(wr_buf);
	kfree(r_buf);
	*/
	return 0;
err_regdevice:	
	misc_deregister(&sca8i2c_device);
err_add:	
	cdev_del(&sca8i2c_devp->dev);
err_alloc:
	unregister_chrdev_region(devno, 0);
    
	return err;
}

static int sca8i2c_remove(struct i2c_client *client)
{
  misc_deregister(&sca8i2c_device);
	cdev_del(&sca8i2c_devp->dev);
	kfree(sca8i2c_devp);
	unregister_chrdev_region(devno, 0);	
  i2c_unregister_device(client);
	return 0;
}

/*-------------------------------------------------------------------------*/

static struct i2c_driver sca8i2c_driver = {
	.driver = {
		.name = "sca8i2c",
		.of_match_table = sca8i2c_of_match,
	},
	.probe = sca8i2c_probe,
	.remove = sca8i2c_remove,
	.id_table = sca8i2c_i2c_id,
};
//module_i2c_driver(sca8i2c_driver);

static int __init sca8i2c_init(void)
{
	return i2c_add_driver(&sca8i2c_driver);
}
module_init(sca8i2c_init);

static void __exit sca8i2c_exit(void)
{
	i2c_del_driver(&sca8i2c_driver);
}
module_exit(sca8i2c_exit);

MODULE_DESCRIPTION("Driver for Sensory EEPROM");
MODULE_AUTHOR("MediaTek Inc.");
MODULE_LICENSE("GPL");
