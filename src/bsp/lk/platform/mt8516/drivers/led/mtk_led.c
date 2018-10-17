#include <stdio.h>
#include <platform/mtk_i2c.h>
#include <platform/mtk_timer.h>
u16 busnum = 1 ;
u8 dev_addr[4] = {0x32,0x33,0x34,0x35};
u8 wr_buf_2[2];
u8 wr_buf_1 = 0;
u32 wr_len_2 = 2;
u32 wr_len_1 = 1;
u32 speed = 100;
u32 rd_len = 1;
u8 rd_buf = 0;
#define LP5523_REG_ENABLE		    0x00
#define LP5523_REG_OP_MODE		    0x01
#define LP5523_REG_ENABLE_LEDS_MSB	0x04
#define LP5523_REG_ENABLE_LEDS_LSB	0x05
#define LP5523_REG_LED_PWM_BASE		0x16
#define LP5523_REG_LED_CURRENT_BASE	0x26
#define LP5523_REG_CONFIG		    0x36
#define LP5523_ENABLE			    0x40
#define LP5523_AUTO_INC			    0x40
#define LP5523_PWR_SAVE			    0x20
#define LP5523_PWM_PWR_SAVE		    0x04
#define LP5523_CP_AUTO			    0x18
#define LP5523_AUTO_CLK			    0x02
#define LP5523_MAX_LEDS			    9
#define DEVICE_NUM                  4
#define led_cur                     0x20
#define max_chan                    0x09
/**********************************************/
//MODE:     GPIO_BASE+0X3D0[14:12] //000 gpio mode
//DIR:        GPIO_BASE+0X040[5]
//DATAOUT:GPIO_BASE+0X140[5]
/**********************************************/
static void gpio69_set_enable(void)
{
  unsigned int tmp = (unsigned int)(*(volatile unsigned int *)(IO_PHYS + 0x00053D0));
  tmp &= ~(0x700);
  (*(volatile unsigned int *)(IO_PHYS + 0x00053D0)) = (unsigned int)tmp;
  tmp = (unsigned int)(*(volatile unsigned int *)( IO_PHYS + 0x0005040));
  tmp |= 0x20;
  (*(volatile unsigned int *)( IO_PHYS + 0x0005040)) = (unsigned int)tmp;
  tmp = (unsigned int)(*(volatile unsigned int *)( IO_PHYS + 0x0005140));
  tmp &= ~(0x20);
  (*(volatile unsigned int *)( IO_PHYS + 0x0005140)) = (unsigned int)tmp;
  udelay(2000);
  tmp = (unsigned int)(*(volatile unsigned int *)( IO_PHYS + 0x0005140));
  tmp |= 0x20;
  (*(volatile unsigned int *)( IO_PHYS + 0x0005140)) = (unsigned int)tmp;
  udelay(2000);
}

static int lp55xx_detect_device(int device_idx)
{
  int ret;
  wr_buf_2[0] = LP5523_REG_ENABLE;
  wr_buf_2[1] = LP5523_ENABLE;
  ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  if (ret)
    return ret;
  udelay(2000);
  wr_buf_1 = LP5523_REG_ENABLE;
  ret = mtk_i2c_write_read(busnum,dev_addr[device_idx],speed,&wr_buf_1,&rd_buf,wr_len_1,rd_len);
  if (ret)
  	return ret;
  if(rd_buf != LP5523_ENABLE)
  {
    return -1;
  }
  return 0;
}

static inline void lp5523_wait_opmode_done(void)
{
  udelay(2000);
}

static void lp5523_set_led_current(int device_idx)
{
  int i=0;
  for(i=0;i<max_chan;i++)
  {
    wr_buf_2[0] = LP5523_REG_LED_CURRENT_BASE + i;
	wr_buf_2[1] = led_cur;
	mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  }
}

static void lp5523_stop_all_engines(int device_idx)
{
  wr_buf_2[0] = LP5523_REG_OP_MODE;
  wr_buf_2[1] = 0;
  mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  lp5523_wait_opmode_done();
}

static int lp55xx_init(int device_idx)
{
  int ret;
  wr_buf_2[0] = LP5523_REG_ENABLE;
  wr_buf_2[1] = LP5523_ENABLE;
  ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  if (ret)
    return ret;
  udelay(2000);
  wr_buf_2[0] = LP5523_REG_CONFIG;
  wr_buf_2[1] = LP5523_AUTO_INC | LP5523_PWR_SAVE | LP5523_CP_AUTO | LP5523_AUTO_CLK | LP5523_PWM_PWR_SAVE;
  ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  if (ret)
  	return ret;
  wr_buf_2[0] = LP5523_REG_ENABLE_LEDS_MSB;
  wr_buf_2[1] = 0x01;
  ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  if (ret)
	return ret;
  wr_buf_2[0] = LP5523_REG_ENABLE_LEDS_LSB;
  wr_buf_2[1] = 0xff;
  ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  if (ret)
	return ret;
  return 0;
}

static int lp5523_pwm_light(int device_idx)
{
  int i = 0;
  int ret;
  lp5523_stop_all_engines(device_idx);
  wr_buf_2[0] = LP5523_REG_ENABLE_LEDS_MSB;
  wr_buf_2[1] = 0x01;
  ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  if (ret)
  	return ret;
  wr_buf_2[0] = LP5523_REG_ENABLE_LEDS_LSB;
  wr_buf_2[1] = 0xFF;
  ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
  if (ret)
  	return ret;
  for (i = 0; i < LP5523_MAX_LEDS; i++)
  {
    wr_buf_2[0] = LP5523_REG_LED_PWM_BASE + i;
    wr_buf_2[1] = 0xff;
	ret = mtk_i2c_write(busnum,dev_addr[device_idx],speed,wr_buf_2,wr_len_2);
	if (ret)
		return ret;
	udelay(4000);
  }
  return ret;
}

int led_init(void)
{
  int ret;
  int i=0;
  gpio69_set_enable();
  for(i=0;i<DEVICE_NUM;i++)
  {
	ret = lp55xx_detect_device(i);
	if(ret)
	{
      return -1;
	}
    ret = lp55xx_init(i);
	if(ret)
	{
      return -2;
	}
	lp5523_set_led_current(i);
  }
  for(i=0;i<DEVICE_NUM;i++)
  {
    lp5523_pwm_light(i);
  }
  return ret;
}
